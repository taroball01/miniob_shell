#include "optimizer/optimizer.h"
#include <memory>
#include <stdexcept>
#include "common/dynamic_unique_cast.h"
#include "planner/plan_node.h"
#include "sql/predicate/predicate.h"
namespace query_process_engine {

auto Optimizer::optimize(std::unique_ptr<PlanNode> ptr) -> std::unique_ptr<PlanNode> {
  ptr = choose_join_sequence(std::move(ptr));
  ptr = push_down_filter(std::move(ptr));
  for (auto &opt : custom_optimzers_) {
    ptr = opt(std::move(ptr));
  }
  return ptr;
}

auto Optimizer::register_optimzer(optimize_func_t func) -> void { custom_optimzers_.emplace_back(func); }

auto Optimizer::choose_join_sequence(std::unique_ptr<PlanNode> ptr) -> std::unique_ptr<PlanNode> {
  switch (ptr->get_plan_node_type()) {
    case PlanNodeType::MultiJoin: {
      auto &mtj = dynamic_cast<MultiJoinPlanNode &>(*ptr);
      auto &children = mtj.get_children();
      std::unique_ptr<PlanNode> cur;
      for (auto &child : children) {
        // forget to change self children
        child = choose_join_sequence(std::move(child));
        if (nullptr == cur) {
          cur = std::move(child);
        } else {
          cur = std::make_unique<BinaryJoinPlanNode>(std::move(cur), std::move(child));
        }
      }
      return cur;
    }
    case PlanNodeType::TableScan: {
      return ptr;
    }
    case PlanNodeType::Filter: {
      auto &flt = dynamic_cast<FilterPlanNode &>(*ptr);
      flt.set_child(choose_join_sequence(flt.transfer_child()));
      return ptr;
    }
    case PlanNodeType::Projection: {
      auto &prj = dynamic_cast<ProjectionPlanNode &>(*ptr);
      prj.set_child(choose_join_sequence(prj.transfer_child()));
      return ptr;
    }
    case PlanNodeType::BinaryJoin: {
      auto &bj = dynamic_cast<BinaryJoinPlanNode &>(*ptr);
      bj.set_lchild(choose_join_sequence(bj.transfer_lchild()));
      bj.set_rchild(choose_join_sequence(bj.transfer_rchild()));
      return ptr;
    }
    default: {
      return ptr;
      // throw std::logic_error("Optimizer::choose_join_sequence");
    }
  }
}

auto Optimizer::push_down_filter(std::unique_ptr<PlanNode> ptr) -> std::unique_ptr<PlanNode> {
  join_trie_t join_trie;
  build_join_trie(*ptr, join_trie);
  filter_arr_t filter_arr;
  return push_down_filter(std::move(ptr), join_trie, filter_arr);
}

auto Optimizer::build_join_trie(PlanNode &node, join_trie_t &join_trie) -> const PlanNode * {
  switch (node.get_plan_node_type()) {
    case PlanNodeType::TableScan: {
      auto &ts = dynamic_cast<TableScanPlanNode &>(node);
      return &ts;
    }
    case PlanNodeType::Filter: {
      auto &flt = dynamic_cast<FilterPlanNode &>(node);
      return build_join_trie(flt.get_child(), join_trie);
    }
    case PlanNodeType::Projection: {
      auto &prj = dynamic_cast<ProjectionPlanNode &>(node);
      return build_join_trie(prj.get_child(), join_trie);
    }
    case PlanNodeType::BinaryJoin: {
      auto &bj = dynamic_cast<BinaryJoinPlanNode &>(node);
      auto l_ret = build_join_trie(bj.get_lchild(), join_trie);
      auto r_ret = build_join_trie(bj.get_rchild(), join_trie);

      auto build_from_child = [&join_trie, &bj](const PlanNode *ch, ChildDirection direction) {
        if (ch->get_plan_node_type() == PlanNodeType::TableScan) {
          auto ts = dynamic_cast<const TableScanPlanNode *>(ch);
          join_trie[&bj][ts->get_relation()] = direction;
        } else {
          auto ch_bj = dynamic_cast<const BinaryJoinPlanNode *>(ch);
          for (auto &[k, _] : join_trie.at(ch_bj)) {
            join_trie[&bj][k] = direction;
          }
        }
      };
      build_from_child(l_ret, ChildDirection::Left);
      build_from_child(r_ret, ChildDirection::Right);
      return &bj;
    }
    default: {
      return nullptr;
      // throw std::logic_error("Optimizer::build_join_trie");
    }
  }
}

auto Optimizer::build_relations_from_predicate(Predicate &pred, std::vector<std::string> &rels) -> void {
  switch (pred.get_node_type()) {
    case PredicateType::CNT_LEAF: {
      auto &leaf = dynamic_cast<PredicateLeaf &>(pred);
      auto build_from_operand = [&rels](Operand &oprd) -> void {
        if (oprd.get_operand_type() == OperandType::COT_ATTR) {
          auto &attr = oprd.get_as<OperandAttr>().get_attr();
          assert(!attr.get_relation().empty());
          rels.emplace_back(attr.get_relation());
        }
      };
      build_from_operand(leaf.get_lchild());
      build_from_operand(leaf.get_rchild());
      break;
    }
    case PredicateType::CNT_AND: {
      auto &and_pred = dynamic_cast<PredicateAnd &>(pred);
      build_relations_from_predicate(and_pred.get_lchild(), rels);
      build_relations_from_predicate(and_pred.get_rchild(), rels);
      break;
    }

    case PredicateType::CNT_NOT: {
      auto &not_pred = dynamic_cast<PredicateNot &>(pred);
      build_relations_from_predicate(not_pred.get_child(), rels);
      break;
    }

    case PredicateType::CNT_OR: {
      auto &or_pred = dynamic_cast<PredicateOr &>(pred);
      build_relations_from_predicate(or_pred.get_lchild(), rels);
      build_relations_from_predicate(or_pred.get_rchild(), rels);
      break;
    }

    default: {
      return;
    }
  }
}

auto Optimizer::check_direction(const std::vector<std::string> &relations,
                                const std::unordered_map<std::string, ChildDirection> &directions) -> ChildDirection {
  ChildDirection dir = ChildDirection::Invalid;
  for (auto &rel : relations) {
    ChildDirection cur_d = directions.at(rel);
    if (dir == ChildDirection::Invalid) {
      dir = cur_d;
    } else if (dir != cur_d) {
      return ChildDirection::Invalid;
    }
  }
  return dir;
}

auto Optimizer::push_down_filter(std::unique_ptr<PlanNode> pnode, const join_trie_t &join_trie,
                                 filter_arr_t &filter_arr) -> std::unique_ptr<PlanNode> {
  switch (pnode->get_plan_node_type()) {
    case PlanNodeType::TableScan: {
      // only for table scan?
      for (auto &filter : filter_arr) {
        filter.first->set_child(std::move(pnode));
        pnode = std::move(filter.first);
      }
      return pnode;
    }
    case PlanNodeType::Filter: {
      auto &cur = dynamic_cast<FilterPlanNode &>(*pnode);
      std::vector<std::string> rel_arr;
      build_relations_from_predicate(cur.get_condition(), rel_arr);
      auto child = cur.transfer_child();

      filter_arr.emplace_back(dynamic_unique_cast<FilterPlanNode>(std::move(pnode)), std::move(rel_arr));
      return push_down_filter(std::move(child), join_trie, filter_arr);
    }
    case PlanNodeType::Projection: {
      auto &p_cur = dynamic_cast<ProjectionPlanNode &>(*pnode);
      p_cur.set_child(push_down_filter(p_cur.transfer_child(), join_trie, filter_arr));
      return pnode;
    }
    case PlanNodeType::BinaryJoin: {
      auto *p_cur = dynamic_cast<BinaryJoinPlanNode *>(pnode.get());
      auto &mp = join_trie.at(p_cur);
      filter_arr_t lft, rgt;
      std::vector<std::unique_ptr<FilterPlanNode>> stay;

      for (auto &[filter, relations] : filter_arr) {
        auto dir = check_direction(relations, mp);
        switch (dir) {
          case ChildDirection::Invalid:
            stay.emplace_back(std::move(filter));
            break;
          case ChildDirection::Left:
            lft.emplace_back(std::move(filter), std::move(relations));
            break;
          case ChildDirection::Right:
            rgt.emplace_back(std::move(filter), std::move(relations));
            break;
        }
      }
      p_cur->set_lchild(push_down_filter(p_cur->transfer_lchild(), join_trie, lft));
      p_cur->set_rchild(push_down_filter(p_cur->transfer_rchild(), join_trie, rgt));
      // stay part
      for (auto &filter : stay) {
        filter->set_child(std::move(pnode));
        pnode = std::move(filter);
      }
      return pnode;
    }
    default: {
      return pnode;
      // throw std::logic_error("Optimizer::push_down_filter");
    }
  }
}
}  // namespace query_process_engine
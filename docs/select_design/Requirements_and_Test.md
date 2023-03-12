### Requirements

查询执行引擎（下称“引擎”）需要满足的需求。

对于语句

`select ATTRS from RELS where CONDS;`

| ID    | name     | Description                                                  |
| ----- | -------- | ------------------------------------------------------------ |
| SL_01 | 数据类型 | 引擎需要支持**字符串**、**整型**和**时间类型**               |
| SL_02 | 属性列表 | ATTRS中的属性只能是不含其它运算的简单属性                    |
| SL_03 | 关系列表 | RELS 可支持单表查询，也可表示为两个表的等值连接；暂时只支持按表名查找，不支持子查询等。 |
| SL_04 | 条件列表 | 1. 引擎需要至少支持 `=`,可扩充至对 `>=`, `<=`, `>`,`<`, `!=` 的支持; 2. 比较符号两边均为简单数值或简单的属性; 3. 引擎至少支持`and` 连接符，可扩充至对`or` 和 `not`的支持 |



### Test Cases

SL_01:

```sql
select name from student;
select birthday from student;
select stuId from student;
select name, birthday from student;
select stuId from student;
select * from student;
```

SL_02:

```sql
# not supported
select name - 1 from student;
```

SL_03:

```sql
select courseName from course where credit < 3.0;
select name from student where birthday > '2001-12-06';
select teacherName from teacher where faculty = 'cs';

select courseName from enroll, course where enroll.stuId = '.';
select stuName from grade, student where courseId = '.' and  score > 90;

select teacherName, courseName, credit from course, teach, teacher where credit > 3.0 or teacher.workingyears > 10;
```

SL_04:

1. `char`只支持`=`, `!=` ， `int` ， `date` 和 `float` 支持 六种操作

   ```sql
   course.credit op 2.0;
   student.birthday op '2001-12-06';
   workingyears op 10;
   teacher.faculty =/!= 'cs';
   ```

2. `and`、`or`和 `not`的优先级与结合性

   ```sql
   # or < and < not
   not A and B;
   (not A) and B;
   
   not A or B;
   (not A) or B;
   
   A and B or C;
   (A and B) or C;
   
   A or B and C;
   A or (B and C);
   ```

   


USE vk

drop view if exists `avg_first_name`;
create view avg_first_name as 
	select first_name
	from user
	where deactivated <> "DELETED"
	group by first_name
	order by count(*) desc
	limit 1;

drop view if exists `avg_last_name`;
create view avg_last_name as
	select last_name
	from user
	where deactivated <> "DELETED"
	group by last_name
	order by count(*) desc
	limit 1;

drop view if exists `avg_sex`;
create view avg_sex as
	select case sex when 1 then "жен" when 2 THEN "муж" else "неопределено" end as sex
	from user
	where deactivated <> "DELETED"
	group by sex
	order by count(*) desc
	limit 1;

drop view if exists `avg_smoking`;
create view avg_smoking as
	select case smoking when 1 then "резко негативное" when 2 THEN "негативное" when 3 THEN "нейтральное" when 4 THEN "компромиссное" when 5 THEN "положительное" else "неопределено" end as smoking	
	from user
	where deactivated <> "DELETED"
	group by smoking
	order by count(*) desc
	limit 1;

drop view if exists `avg_alcohol`;
create view avg_alcohol as
	select case alcohol when 1 then "резко негативное" when 2 THEN "негативное" when 3 THEN "нейтральное" when 4 THEN "компромиссное" when 5 THEN "положительное" else "неопределено" end as alcohol	
	from user
	where deactivated <> "DELETED"
	group by alcohol
	order by count(*) desc
	limit 1;

create temporary table avg_groups as
	select round(sum(sub.count)/count(*)) as groups
	from (select count(*) as count
		  from user_subscriptions
		  where uid is not null and guid is not null
		  group by uid) as sub;


select first_name, last_name, sex, smoking, alcohol, groups
from avg_first_name, avg_last_name, avg_sex, avg_smoking, avg_alcohol, avg_groups


use vk 

drop view if exists group_name;
create view group_name as
	select guid, first_name, count(*) count
	from user_subscriptions right join user using(uid)
	where guid is not null
	group by guid, first_name;


drop view if exists group_religion;
create view group_religion as
	select guid, religion, count(*)/(select count(*) from user_subscriptions right join user using(uid) where guid = US.guid) percentage
	from user_subscriptions US right join user U using(uid)
	where guid is not null
	group by guid, religion;


select G.title, SUB.users, (select first_name
							from group_name
							where guid = G.guid
			   				order by count desc
			   				limit 0,1) top1_name,
			  			   (select first_name
			   				from group_name
			   				where guid = G.guid
			   				order by count desc
			   				limit 1,1) top2_name,
			  			   (select religion
			   				from group_religion
			   				where guid = G.guid
			   				order by percentage desc
			   				limit 0,1) top1_religion,
			  			   (select religion
			   				from group_religion
			   				where guid = G.guid
			   				order by percentage desc
			   				limit 1,1) top2_religion

from `group` G inner join (select guid, count(*) users
						   from user_subscriptions
						   where uid in not null
 						   group by guid 
 						   order by count(*) desc) SUB using(guid)
order by SUB.users desc





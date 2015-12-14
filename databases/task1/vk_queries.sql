USE vk


SELECT uid,first_name,last_name 
FROM user
WHERE first_name LIKE "З%" AND political IS NULL AND religion IS NOT NULL



SELECT first_name 
FROM user 
WHERE first_name <> "DELETED"
GROUP BY first_name
ORDER BY COUNT(*) DESC
LIMIT 1



SELECT G.title, US.guid
FROM `group` G INNER JOIN ( SELECT guid
					 		FROM user_subscriptions 
					 		GROUP BY guid
					 		ORDER BY COUNT(*) DESC
					 		LIMIT 3) US ON G.guid=US.guid



SELECT G.title
FROM `group` G INNER JOIN ( SELECT US.guid, sum(IF(U.sex=1,1,0)) women, sum(IF(U.sex=2,1,0)) men
							FROM user_subscriptions US INNER JOIN user U ON US.uid = U.uid
							GROUP BY US.guid) SUB ON G.guid = SUB.guid
WHERE SUB.women !=0
ORDER BY (SUB.men/(SUB.men+SUB.women)) DESC
LIMIT 10

#!/sbin/env python3

import json
import random
from sys import exit
from urllib.request import urlopen
from urllib.error import HTTPError
from collections import defaultdict

VK_USERS = 270000000

student_table = {'p': [0.800, 0.900, 0.950, 0.980, 0.990, 0.995, 0.998, 0.999],
				 't': [1.283, 1.647, 1.964, 2.333, 2.785, 2.819, 3.106, 3.310]}



class VkClient(object):

	def __init__(self, api_url='http://api.vk.com/method/', format='json'):
		if format not in ['json', 'xml']:
			raise VkException('format must be json or xml')
		self.api_url = api_url
		self.format = format

	def request(self, method, params):
		url = self.api_url + method + '.' + self.format + '?' + '&'.join([str(key) + '=' + str(value) for key, value in params.items()])
		data = json.loads(urlopen(url).read().decode('utf8'))
		if 'error' in data:
			raise VkException(date['error']['error_msg'])
		return data

	def getUserSubscription(self, user_id):
		params = {'user_id': user_id}
		method = 'users.getSubscriptions'
		return self.request(method, params)
 
	def getUserInfo(self, user_id, fields=''):
		params = {'user_ids': user_id,
				  'fields':   fields}
		method = 'users.get'
		return self.request(method, params)



class VkAnalyzer(object):

	def __init__(self):
		self.vk_client = VkClient()
		self.users = defaultdict(int)

	def getSampleSize(self, number, error=0.05, reliability=0.95, p=0.5):
		z = student_table['t'][student_table['p'].index(min(student_table['p'], key=lambda x:abs(x-reliability)))]
		return int((z**2*p*(1-p)*number)/(error**2*number+z**2*p*(1-p)))
		
	def getTopUsers(self, users_count, error=0.05, reliability=0.95):			
		for user_id in random.sample(range(VK_USERS), self.getSampleSize(VK_USERS, error, reliability)):
			try:
				for item in self.vk_client.getUserSubscription(user_id)['response']['users']['items']:
					self.users[item] += 1
			except HTTPError as e:
				print(e)
		return sorted(self.users, key=self.users.get, reverse=True)[0:users_count]



class VkException(Exception):
	pass



def printUserInfo(users):
	vk_client = VkClient()		
	print('\n{0:>10}  {1:>10}  {2:10}  {3:15}'.format('N', 'id', 'name', 'surname'))	
	for num, user in enumerate(users, start=1):			
		info = vk_client.getUserInfo(user)['response'][0]		
		print('{num:10}  {uid:10}  {first_name:10}  {last_name:15}'.format(num=num, **info))				



def main():	
	vk_analyzer = VkAnalyzer()	
	try:
		users = vk_analyzer.getTopUsers(30, error=0.05, reliability=0.95)
		printUserInfo(users)
	except (VkException, HTTPError) as e:
		print(e) 
		exit(1)

if __name__ == '__main__':
	main()
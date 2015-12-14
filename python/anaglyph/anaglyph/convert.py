#!/usr/bin/env python2.7

from argparse import ArgumentParser
from abc import ABCMeta, abstractmethod
from os import path
from PIL import Image
from cv2 import imread, imwrite, cvtColor, COLOR_BGR2RGB, COLOR_RGB2BGR
from cv2 import error as CVError
import numpy
import json


CONFIG = path.join(path.dirname(__file__), "config.json")


class BaseConverter(object):

	__metaclass__ = ABCMeta	

	def __init__(self, config_file_name=CONFIG, method="optimized"):
		try:					
			config = json.load(open(config_file_name))
			self.m1 = config['transform']['methods'][method][0]
			self.m2 = config['transform']['methods'][method][1]
		except IOError:
			raise ConverterConfigException("config file can't be opened")
		except KeyError:
			raise ConverterConfigException("config file format incorrect")

	@abstractmethod
	def get_anaglyph(self, left_image_name, right_image_name, out_image_name):
		pass

	@abstractmethod
	def convert(self, left_matrix, right_matrix):
		pass




class NativeConverter(BaseConverter):

	def get_anaglyph(self, left_image_name, right_image_name, out_image_name):
		left_image = Image.open(left_image_name)
		right_imge = Image.open(right_image_name)
		result_image = Image.new("RGB", left_image.size)

		result_image.putdata(self.convert(left_image.getdata(), right_imge.getdata()))
		result_image.save(out_image_name, "JPEG")

		
		
	def convert(self, left_matrix, right_matrix):
		if (len(left_matrix), len(left_matrix[0])) != (len(right_matrix), len(right_matrix[0])):
			raise ConverterImageException("images size must be equal")	

		return [ tuple(self.vector_add(self.matrix_mult(self.m1, self.transp(  [left_matrix[m]] )), 
								       self.matrix_mult(self.m2, self.transp( [right_matrix[m]] ))
								      )
					  ) for m in xrange(len(left_matrix)) ]
	
	@staticmethod
	def transp(matrix):
		return [ [matrix[m][n] for m in xrange(len(matrix))] for n in xrange(len(matrix[0])) ] 

	@staticmethod
	def matrix_mult(matrix1, matrix2):
		if (len(matrix1[0]) != len(matrix2)):
			raise ConverterImageException("multiplication matrix dimention error")
		
		return [ [sum( [matrix1[m][n]*matrix2[n][k] for n in xrange(len(matrix2))] )  
													for k in xrange(len(matrix2[0])) ]  
													for m in xrange(len(matrix1)) ]		
	
	@staticmethod
	def vector_add(matrix1, matrix2):		
		if (len(matrix1), len(matrix1[0])) != (len(matrix2), len(matrix2[0])):
			raise ConverterImageException("additon matrix dimention error")
	
		return [ int(c1+c2) for m in xrange(len(matrix1)) for (c1,c2) in zip(matrix1[m], matrix2[m]) ] 




class FastConverter(BaseConverter):	

	def get_anaglyph(self, left_image_name, right_image_name, out_image_name):
		left_matrix = cvtColor(imread(left_image_name, 1), COLOR_BGR2RGB)
		right_matrix = cvtColor(imread(right_image_name, 1), COLOR_BGR2RGB)	
		
		imwrite(out_image_name, cvtColor(numpy.array(self.convert(left_matrix, right_matrix)), COLOR_RGB2BGR))
		
	def convert(self, left_matrix, right_matrix):
		if left_matrix.shape != right_matrix.shape:
			raise ConverterImageException("images size must be equal")			
		
		return numpy.add(numpy.dot(self.m1,  left_matrix.reshape(-1, 3).transpose()), 
						 numpy.dot(self.m2, right_matrix.reshape(-1, 3).transpose())).astype(numpy.uint8).transpose().reshape(left_matrix.shape)
		
	
	





class ConverterException(Exception):
	pass

class ConverterConfigException(ConverterException):
	pass

class ConverterImageException(ConverterException):
	pass



def main():
	parser = ArgumentParser(description="Generate anaglyph from stereo pair.")    
	parser.add_argument("-l", "--left", dest="left", help="left image", metavar="FILE", required=True)
	parser.add_argument("-r", "--right", dest="right", help="right image", metavar="FILE", required=True)
	parser.add_argument("-o", "--out", dest="out", help="out image", metavar="FILE", required=True)	
	parser.add_argument("-m", "--method", dest="method", help="transform method (default: %(default)s)", metavar="METHOD", default="optimized")  
	parser.add_argument("-c", "--config", dest="config", help="config file (default: %(default)s)", metavar="FILE", default=CONFIG)  

	args = parser.parse_args()	

	try:		
		converter = FastConverter(args.config, args.method)		
		converter.get_anaglyph(args.left, args.right, args.out)		

	except (ConverterException, CVError) as e:
		print(e)



if __name__ == '__main__':
	main()
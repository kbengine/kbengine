# -*- coding: utf-8 -*-

"""
用法:
a = 1
b = 2
def abc(a, b):
   print a, b
   
func = Functor(abc, a)
func(b)
"""

class Functor:
	def __init__(self, func, *args):
		self.func = func
		self.args = args

	def __call__(self, *args):
		self.func(*(self.args + args))
		
#!/usr/bin/env python2.7

from distutils.core import setup

setup(name='anaglyph_generator',
      version='1.0',
      description='python anaglyph generator',    
      packages=['anaglyph',],
      requires=['numpy', 'cv2', 'PIL'],
      package_data={'anaglyph': ['config.json']},
     )
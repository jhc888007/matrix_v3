#!/usr/bin/python
# coding=utf-8

from distutils.core import setup, Extension

module = Extension('matrixbuilder7', sources = ['matrixbuilder.cpp'], extra_compile_args = ['-g'])

setup(name = 'matrix_builder_7', version = '1.0', ext_modules = [module])


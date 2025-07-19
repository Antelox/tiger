#!/usr/bin/env python

from setuptools import Extension, setup

tiger = Extension("tiger", sources=["sboxes.c", "tiger.c", "tigermodule.c"])

setup(
    name="tiger",
    version="0.4",
    description="Tiger hash module",
    long_description=open("README.rst").read(),
    author="Corbin Simpson",
    author_email="MostAwesomeDude@gmail.com",
    url="http://github.com/MostAwesomeDude/tiger",
    ext_modules=[tiger],
)

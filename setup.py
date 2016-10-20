#!/usr/bin/env python
from setuptools import setup, Extension


with open('README.rst') as f:
    long_description = f.read()

setup(
    name='cotoolz',
    version='0.1.6',
    description='A set of utility functions for coroutines.',
    author='Joe Jevnik',
    author_email='joejev@gmail.com',
    packages=['cotoolz'],
    include_package_data=True,
    long_description=long_description,
    license='GPL-2',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'Programming Language :: Python :: 3 :: Only',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: Implementation :: CPython',
        'Operating System :: POSIX',
        'Topic :: Software Development',
        'Topic :: Utilities',
    ],
    url='https://github.com/llllllllll/cotoolz',
    ext_modules=[
        Extension(
            'cotoolz._emptycoroutine',
            ['cotoolz/_emptycoroutine.c'],
            include_dirs=['cotoolz/include'],
        ),
        Extension(
            'cotoolz._coiter',
            ['cotoolz/_coiter.c'],
            include_dirs=['cotoolz/include'],
        ),
        Extension(
            'cotoolz._comap',
            ['cotoolz/_comap.c'],
            include_dirs=['cotoolz/include'],
        ),
        Extension(
            'cotoolz._cozip',
            ['cotoolz/_cozip.c'],
            include_dirs=['cotoolz/include'],
        ),
    ],
    install_requires=[
        'toolz>=0.7.2',
    ],
    extras_require={
        'dev': [
            'flake8==2.4.1',
            'pytest==2.7.2',
            'toolz==0.8.0',
        ],
    },
)

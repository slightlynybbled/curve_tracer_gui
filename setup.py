from setuptools import setup, find_packages
import os

# ---------------------------------
# imports the version from the package
here = os.path.dirname(os.path.dirname(__file__))
exec(open(os.path.join(here, 'curve_tracer/version.py')).read())

# ---------------------------------
# converts the readme.md to readme.rst
try:
    from pypandoc import convert
    readme = convert('readme.md', 'rst')
except ImportError:
    readme = ''


# ---------------------------------
# project requirements
requirements = [
    'serialdispatch',
    'pillow'
]

# ---------------------------------
# project setup
setup(
    name='curve_tracer',
    version=__version__,
    description='Curve Tracer',
    long_description=readme,
    author='Jason R. Jones',
    author_email='slightlynybbled@gmail.com',
    url='https://github.com/slightlynybbled/curve_tracer_gui',
    packages=find_packages(),
    entry_points={'console_scripts': [
        'curve_tracer = curve_tracer.__main__:main']
    },
    include_package_data=True,
    install_requires=requirements,
    zip_safe=False,
    license='MIT',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Natural Language :: English'
    ],
    keywords='electronics curve tracer huntron'
)

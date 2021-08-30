# MDOODZ6.0 with the web interface
## Overview
This project is the extention of the Thibault Duretz C code https://github.com/tduretz/MDOODZ6.0/
### Consists of 3 systems
- Front-end (Nuxt.js)
- Server (Python flask) 
- MDOODZ6.0 source code (C language)

### Prerequisites
- Node.js & npm
- Python 3.7 & pipenv
- GCC, SuiteSparse, HDF5 (can be installed with Homebrew, apt, apt-get or any other package manager)

### Setup
1) Clone the repository
2) Copy-paste .env.example to .env - this is where all environmental variables are stored. You will need to put yours system's `COMPILER_LOCATION`, `C_INCLUDE_PATH` and `LIBRARY_PATH`  
3) Install python modules with `pipenv install`
4) Install js modules with `npm ci`

### Run in development mode
For server:
```bash
flask run
```
For client:
```bash
npm run dev
```
Client will be available at the address http://localhost:3000/

## Development
### Code style
- Python PEP8 style guide https://www.python.org/dev/peps/pep-0008/
- For C we may use Google C++ Style guide: https://google.github.io/styleguide/cppguide.html
- For Nuxt.js project will be implemanted linted

Please note that aligning variable declarations with the whitespaces considered to be a bad practice. Always use one whitespace before and after `=`

#### Variable namings
- For С and Python variables use `snake_case`, for JavaScript `camelCase`
- For DTO (Data Transfer Objects) `snake_case` is used always




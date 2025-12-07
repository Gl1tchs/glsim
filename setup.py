from skbuild import setup

setup(
    name="glsim",
    version="0.1.0",
    packages=["glsim"],
    # scikit-build automatically looks for CMakeLists.txt 
    # and runs 'cmake .. && make' 
    cmake_args=[
        '-DCMAKE_BUILD_TYPE=Release', 
    ]
)

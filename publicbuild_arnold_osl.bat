mkdir build_public
cd build_public
cmake -DMATERIALX_BUILD_PYTHON=ON -DMATERIALX_WARNINGS_AS_ERRORS=ON -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Debug -DMATERIALX_BUILD_DOCS=ON -DMATERIALX_BUILD_VIEW=ON -DMATERIALX_TEST_VIEW=ON -DMATERIALX_OSLC_EXECUTABLE="d:/Work/arnold/Arnold-SDK/bin/oslc.exe" -DMATERIALX_OSL_INCLUDE_PATH="d:/Work/arnold/Arnold-SDK/osl/include" ..
cmake --build . --target MaterialXDocs --config Debug
cmake --build . --target install --config Debug
ctest --VV --output-on-failure --build-config Debug
cmake -E chdir ../python/MaterialXTest python main.py
cd ..

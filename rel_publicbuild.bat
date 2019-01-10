mkdir build_public_rel
cd build_public_rel
cmake -DMATERIALX_BUILD_PYTHON=ON -DMATERIALX_WARNINGS_AS_ERRORS=ON -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Debug -DMATERIALX_BUILD_DOCS=ON -DMATERIALX_BUILD_RENDER=ON -DMATERIALX_TEST_RENDER=ON -DMATERIALX_OSLC_EXECUTABLE="D:/Work/materialx/osl_from_max/OSL_runnable/bin/Release/oslc.exe" -DMATERIALX_TESTSHADE_EXECUTABLE="D:/Work/materialx/osl_from_max/OSL_runnable/bin/Release/testshade" -DMATERIALX_TESTRENDER_EXECUTABLE="D:/Work/materialx/osl_from_max/OSL_runnable/bin/Release/testrender" -DMATERIALX_OSL_INCLUDE_PATH="D:/Work/materialx/osl_from_max/OSL_runnable/shaders" ..
cmake --build . --target MaterialXDocs --config RelWithDebInfo
cmake --build . --target install --config RelWithDebInfo
ctest -VV --output-on-failure --build-config RelWithDebInfo
cmake -E chdir ../python/MaterialXTest python main.py
cd ..

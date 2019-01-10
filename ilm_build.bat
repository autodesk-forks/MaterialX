 mkdir build_public_ilm
 cd build_public_ilm
 cmake -DMATERIALX_BUILD_PYTHON=ON -DMATERIALX_WARNINGS_AS_ERRORS=ON -DMATERIALX_BUILD_DOCS=ON -G "Visual Studio 14 2015 Win64" ..
 cmake --build . --target MaterialXDocs --config Debug  
 cmake --build . --target install --config Debug
 ctest --verbose --output-on-failure --build-config Debug
 cmake -E chdir ../python/MaterialXTest python main.py
 cd ..
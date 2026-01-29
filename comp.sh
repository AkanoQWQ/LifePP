mkdir -p build
g++ smana/smana.cpp -o build/smana -mconsole -Icommon -Itools
g++ server/lifepp-server.cpp -o build/lifepp-server -mwindows -Icommon -Itools
g++ server/lifepp-server.cpp -o build/lifepp-server-DEBUG -mconsole -Icommon -Itools
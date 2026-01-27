mkdir -p build
g++ smana/smana.cpp -o build/smana -mconsole -municode -Icommon -Itools
g++ server/lifepp-server.cpp -o build/lifepp-server -mwindows -municode -Icommon -Itools
g++ server/lifepp-server.cpp -o build/lifepp-server-DEBUG -mconsole -municode -Icommon -Itools
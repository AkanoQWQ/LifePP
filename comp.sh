mkdir -p build
g++ smana/smana.cpp -o build/smana -mconsole -municode -Icommon -Itools
g++ server/lifepp-server.cpp -o build/lifepp-server -mconsole -municode -Icommon -Itools
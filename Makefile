all:
	g++ -o main main.cpp -F/Library/Frameworks/ -framework SDL2
	./main
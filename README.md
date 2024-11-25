all:
	g++ -o main project150_snake.cpp -F/Library/Frameworks/ -framework SDL2 -framework SDL2_ttf -framework SDL2_image -framework SDL2_mixer
	./main
 

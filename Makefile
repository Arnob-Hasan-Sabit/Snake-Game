all:
	g++ -o main main.cpp -F/Library/Frameworks/ -framework SDL2 -framework SDL2_ttf -framework SDL2_image -framework SDL2_mixer
	./main


	g++ -o main st.cpp -F/Library/Frameworks/ -framework SDL2 -framework SDL2_ttf -framework SDL2_image -framework SDL2_mixer

# Usage:
# make        # compile all binary
# make clean  # remove ALL binaries and objects

OBJ=glad.o imgui.o imgui_impl_glfw.o imgui_impl_opengl3.o imgui_draw.o imgui_widgets.o imgui_tables.o main.o
INCLUDE=-I/home/stuart/lib/imgui -I/home/stuart/lib/stb -I/home/stuart/lib/glad -I/home/stuart/lib/imgui/backend
LIB=-lGL -lglfw -ldl -lstdc++ -lm

all: lgl
	gcc $(OBJ) -o lgl $(LIB)

lgl: $(OBJ)

glad.o: ../../lib/glad/glad.c
	gcc -c -o glad.o ../../lib/glad/glad.c $(INCLUDE)

main.o: main.cpp
	gcc -c -o main.o main.cpp $(INCLUDE)

imgui.o: ../../lib/imgui/imgui.cpp
	gcc -c -o imgui.o ../../lib/imgui/imgui.cpp $(INCLUDE)

imgui_impl_glfw.o: ../../lib/imgui/backends/imgui_impl_glfw.cpp
	gcc -c -o imgui_impl_glfw.o ../../lib/imgui/backends/imgui_impl_glfw.cpp $(INCLUDE)

imgui_impl_opengl3.o: ../../lib/imgui/backends/imgui_impl_opengl3.cpp
	gcc -c -o imgui_impl_opengl3.o ../../lib/imgui/backends/imgui_impl_opengl3.cpp $(INCLUDE)

imgui_draw.o: ../../lib/imgui/imgui_draw.cpp
	gcc -c -o imgui_draw.o ../../lib/imgui/imgui_draw.cpp $(INCLUDE)

imgui_widgets.o: ../../lib/imgui/imgui_widgets.cpp
	gcc -c -o imgui_widgets.o ../../lib/imgui/imgui_widgets.cpp $(INCLUDE)

imgui_tables.o: ../../lib/imgui/imgui_tables.cpp
	gcc -c -o imgui_tables.o ../../lib/imgui/imgui_tables.cpp $(INCLUDE)

clean:
	@echo "Cleaning up..."
	rm -rvf lgl
	rm -rvf *.o

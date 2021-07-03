
#include <string.h>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

#define GLCHKERR(f) {GLenum e=glGetError();if(e!=GL_NO_ERROR)printf("GLERR:%i fn:" f "\n",e);}
#define DEG2RAD(d) ((d/180.0f)*3.14159f)

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")

using namespace glm;

bool rayTriangleIntersect( 
    const vec3 &orig, const vec3 &dir, 
    const vec3 &v0, const vec3 &v1, const vec3 &v2, 
    float &t);

GLuint vsh = 0, fsh = 0, prog = 0;
GLuint vbo_vertices = 0, vbo_instances = 0;
GLuint vao = 0;
GLuint texid = 0;
float azimuth = 45.0f;
float elevation = 90.0f;
vec3 upos = vec3(-2.0f, 0.5f, -2.0f);

// x = r cos Φ sin Θ
// y = r sin Φ sin Θ
// z = r cos Θ
inline void sph2cart(float theta, float phi, vec3& cartesian)
{
	float phi_radians = DEG2RAD(phi);
	float theta_radians = DEG2RAD(theta);
	cartesian.x = cos(theta_radians) * sin(phi_radians);
	cartesian.y = cos(phi_radians);
	cartesian.z = sin(theta_radians) * sin(phi_radians);
}

#define NUM_BLOCK_VERTICES 36
size_t NumberOfInstances = 0;

typedef struct _INSTANCE_TYPE {
	glm::vec3 translation;
} INSTANCE_TYPE, * LP_INSTANCE_TYPE;

uint8_t instances[16 * 16 * 16];

static const struct {
	glm::vec3 vertex;
	glm::vec4 color;
	glm::vec3 normal;
	glm::vec2 texc;
} vertices [] = {
	// front - red
	{ {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, -1.0f }, {0.0f, 0.0f}},
	{ { 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, -1.0f }, {1.0f, 1.0f}},
	{ { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, -1.0f }, {1.0f, 0.0f}},
	{ {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, -1.0f }, {0.0f, 0.0f}},
	{ {0.0f,  1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, -1.0f }, {0.0f, 1.0f}},
	{ { 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, -1.0f }, {1.0f, 1.0f}},
	// back - ?
	{ {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f}},
	{ { 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f }, {1.0f, 0.0f}},
	{ { 1.0f,  1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f }, {1.0f, 1.0f}},
	{ {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f }, {0.0f, 0.0f}},
	{ { 1.0f,  1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f }, {1.0f, 1.0f}},
	{ {0.0f,  1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f }, {0.0f, 1.0f}},
	// top - green
	{ {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 1.0f, 0.0f }, {0.0f, 0.0f}},
	{ { 1.0f, 1.0f,  1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 1.0f, 0.0f }, {1.0f, 1.0f}},
	{ { 1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 1.0f, 0.0f }, {1.0f, 0.0f}},
	{ {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 1.0f, 0.0f }, {0.0f, 0.0f}},
	{ {0.0f, 1.0f,  1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 1.0f, 0.0f }, {0.0f, 1.0f}},
	{ { 1.0f, 1.0f,  1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, { 0.0f, 1.0f, 0.0f }, {1.0f, 1.0f}},
	// bottom - cyan
	{ {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f }, {0.0f, 0.0f}},
	{ { 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f }, {1.0f, 0.0f}},
	{ { 1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f }, {1.0f, 1.0f}},
	{ {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f }, {0.0f, 0.0f}},
	{ { 1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f }, {1.0f, 1.0f}},
	{ {0.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, -1.0f, 0.0f }, {0.0f, 1.0f}},
	// front - blue
	{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, {0.0f, 0.0f}},
	{ {0.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, {1.0f, 1.0f}},
	{ {0.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, {1.0f, 0.0f}},
	{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, {0.0f, 0.0f}},
	{ {0.0f, 0.0f,  1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, {0.0f, 1.0f}},
	{ {0.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, { -1.0f, 0.0f, 0.0f }, {1.0f, 1.0f}},
	// back - magenta
	{ {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, {0.0f, 0.0f}},
	{ {1.0f,  1.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, {1.0f, 0.0f}},
	{ {1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, {1.0f, 1.0f}},
	{ {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, {0.0f, 0.0f}},
	{ {1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, {1.0f, 1.0f}},
	{ {1.0f, 0.0f,  1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}, { 1.0f, 0.0f, 0.0f }, {0.0f, 1.0f} }
};

uint8_t get_instance_data(float fx, float fy, float fz)
{
	if(fx < 0.0f || fx >= 16.0f) return 0;
	if(fy < 0.0f || fy >= 16.0f) return 0;
	if(fz < 0.0f || fz >= 16.0f) return 0;
	return instances[((int)fz * 16 * 16) + ((int)fy * 16) + (int)fx];
}

char* read_shader_source(const char* fn)
{
	FILE* f = nullptr;
#ifdef __linux__ 
    //linux code goes here
	f = fopen(fn, "r");
#elif _WIN32
    // windows code goes here
	fopen_s(&f, fn, "r");
#endif
	if (f == nullptr) return nullptr;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* buffer = (char*)malloc(len);
	if (buffer == nullptr) {
		fclose(f);
		return nullptr;
	}
	memset(buffer, 0, len);
	fread(buffer, len, 1, f);
	fclose(f);
	return buffer;
}

int make_program(const char* vfn, const char* ffn, GLuint* vsh, GLuint* fsh, GLuint* prg)
{
	char* vsrc = read_shader_source(vfn);
	if (vsrc == nullptr) {
		printf("failed to read v src\n");
		return 0;
	}
	*vsh = glCreateShader(GL_VERTEX_SHADER);
	GLCHKERR("glCreateShader");
	glShaderSource(*vsh, 1, &vsrc, nullptr);
	GLCHKERR("glShaderSource");
	glCompileShader(*vsh);
	GLCHKERR("glCompileShader");
	GLint compileStatus = 0;
	glGetShaderiv(*vsh, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		glGetShaderiv(*vsh, GL_INFO_LOG_LENGTH, &compileStatus);
		char* infoLogBuffer = (char*)malloc(compileStatus);
		GLsizei len = 0;
		glGetShaderInfoLog(*vsh, compileStatus, &len, infoLogBuffer);
		printf("COMPILE ERROR VERTEX SHADER: %s\n", infoLogBuffer);
		free(infoLogBuffer);
		return 0;
	}
	printf("vsh is %i\n", *vsh);

	char* fsrc = read_shader_source(ffn);
	if (fsrc == nullptr) {
		printf("failed to read f src\n");
		return 0;
	}
	*fsh = glCreateShader(GL_FRAGMENT_SHADER);
	GLCHKERR("glCreateShader");
	glShaderSource(*fsh, 1, &fsrc, nullptr);
	GLCHKERR("glShaderSource");
	glCompileShader(*fsh);
	GLCHKERR("glCompileShader");
	compileStatus = 0;
	glGetShaderiv(*fsh, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		glGetShaderiv(*fsh, GL_INFO_LOG_LENGTH, &compileStatus);
		char* infoLogBuffer = (char*)malloc(compileStatus);
		GLsizei len = 0;
		glGetShaderInfoLog(*fsh, compileStatus, &len, infoLogBuffer);
		printf("COMPILE ERROR FRAGMENT SHADER: %s\n", infoLogBuffer);
		free(infoLogBuffer);
		return 0;
	}
	printf("fsh is %i\n", *fsh);

	*prg = glCreateProgram();
	GLCHKERR("glCreateProgram");
	glAttachShader(*prg, *vsh);
	GLCHKERR("glAttachShader");
	glAttachShader(*prg, *fsh);
	GLCHKERR("glAttachShader");
	glLinkProgram(*prg);
	GLCHKERR("glLinkProgram");
	GLint linkStatus = 0;
	glGetProgramiv(*prg, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_FALSE) {
		glGetProgramiv(*prg, GL_INFO_LOG_LENGTH, &linkStatus);
		char* infoLogBuffer = (char*)malloc(linkStatus);
		GLsizei len = 0;
		glGetProgramInfoLog(*prg, linkStatus, &len, infoLogBuffer);
		printf("LINK ERROR SHADER PROGRAM: %s\n", infoLogBuffer);
		free(infoLogBuffer);
		return 0;
	}
	printf("prg is %i\n", *prg);

	return 1;
}

void setup()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	int result = make_program("vsh.glsl", "fsh.glsl", &vsh, &fsh, &prog);
	
	{
		glGenBuffers(1, &vbo_vertices);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
		GLCHKERR("glBindBuffer");

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		GLCHKERR("glBufferData");

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	std::vector<vec3> vecInst;
	for (int ix = 0; ix < 16; ix++) {
		for (int iz = 0; iz < 16; iz++) {
			float n = stb_perlin_noise3(0.5f + ix, 0.5f + iz, 0.5f, 0, 0, 0);
			for (int iy = 0; iy < 16; iy++) {
				if (iy < (int)(n * 10.0f)) {
					instances[(iz * 16 * 16) + (iy * 16) + ix] = 0x01;
					vecInst.push_back(vec3(ix, iy, iz));
				}
				else {
					instances[(iz * 16 * 16) + (iy * 16) + ix] = 0x00;
				}
			}
		}
	}

	{
		glGenBuffers(1, &vbo_instances);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_instances);
		GLCHKERR("glBindBuffer");

		glBufferData(GL_ARRAY_BUFFER, vecInst.size() * sizeof(INSTANCE_TYPE), 
			vecInst.data(), GL_STATIC_DRAW);
		NumberOfInstances = vecInst.size();
		GLCHKERR("glBufferData");

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);

	glEnableVertexAttribArray(0);
	GLCHKERR("glEnableVertexAttribArray0");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)0);
	GLCHKERR("glVertexAttribPointer0");

	glEnableVertexAttribArray(1);
	GLCHKERR("glEnableVertexAttribArray1");
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	GLCHKERR("glVertexAttribPointer1");

	glEnableVertexAttribArray(2);
	GLCHKERR("glEnableVertexAttribArray2");
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(7 * sizeof(GLfloat)));
	GLCHKERR("glVertexAttribPointer2");

	glEnableVertexAttribArray(3);
	GLCHKERR("glEnableVertexAttribArray2");
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat), (void*)(10 * sizeof(GLfloat)));
	GLCHKERR("glVertexAttribPointer2");

	glBindBuffer(GL_ARRAY_BUFFER, vbo_instances);

	glEnableVertexAttribArray(4);
	GLCHKERR("glEnableVertexAttribArray3");
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	GLCHKERR("glVertexAttribPointer3");
	glVertexAttribDivisor(4, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// texture
	int w, h, c;
	unsigned char* data = stbi_load("texture.png", &w, &h, &c, 0);
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);

	GLCHKERR("setup");
}

void cleanup()
{
	glDeleteBuffers(1, &vbo_vertices);
	glDeleteBuffers(1, &vbo_instances);
	glDeleteShader(vsh);
	glDeleteShader(fsh);
	glDeleteProgram(prog);
}

double oldx, oldy;
int firstReading = 0;
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstReading == 0) {
		firstReading = 1;
		oldx = xpos;
		oldy = ypos;
	}
	else {
		double xd = xpos - oldx;
		double yd = ypos - oldy;
		if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
		{
			azimuth += (float)xd * 0.2f;
			elevation += (float)yd * 0.2f;
			if (elevation < 0.0f) elevation = 0.0f;
			if (elevation > 180.0f) elevation = 180.0f;
		}
		oldx = xpos;
		oldy = ypos;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, 1);
}

int main(int argc, char** argv)
{
	GLFWwindow* window = nullptr;

	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	window = glfwCreateWindow(800, 600, "Hello World", nullptr, nullptr);
	if (!window) {
		printf("ERROR creating glfw window\n");
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);

	if (!gladLoadGL())
	{
		glfwTerminate();
		return -1;
	}

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	printf("Setup...\n");
	setup();

	printf("prog is %i\n", prog);

	double lastTime = glfwGetTime();

	printf("Running...\n");
	while (!glfwWindowShouldClose(window))
	{

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glUseProgram(prog);

		GLint projLoc = glGetUniformLocation(prog, "projection");
		mat4 projMatrix = perspective(45.0f, 640.0f / 480.0f, 0.1f, 1000.0f);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projMatrix));

		// process keys
		vec3 dir;
		vec3 orig = upos;
		sph2cart(azimuth, elevation, dir);
		float velocity_reduction_factor = 10.0f;
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
			velocity_reduction_factor = 50.0f;
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W))
		{			
			upos.x += (dir.x / velocity_reduction_factor);
			upos.z += (dir.z / velocity_reduction_factor);
		}
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S))
		{
			upos.x -= (dir.x / velocity_reduction_factor);
			upos.z -= (dir.z / velocity_reduction_factor);
		}
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A))
		{
			float azleft = azimuth - 90.0f;
			vec3 dirleft;
			sph2cart(azleft, elevation, dirleft);
			upos.x += (dirleft.x / velocity_reduction_factor);
			upos.z += (dirleft.z / velocity_reduction_factor);
		}
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D))
		{
			float azright = azimuth + 90.0f;
			vec3 dirright;
			sph2cart(azright, elevation, dirright);
			upos.x += (dirright.x / velocity_reduction_factor);
			upos.z += (dirright.z / velocity_reduction_factor);
		}
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Q))
		{
			upos.y -= 1 / velocity_reduction_factor;
		}
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Z))
		{
			upos.y += 1 / velocity_reduction_factor;
		}

		int collisionFlag = 0;
		vec3 ipart;
		vec3 fpart = modf(upos, ipart);
		for (int dx=-1; dx<2; dx++) {
			for(int dz=-1; dz<2; dz++ ) {
				vec3 offset(dx, 0, dz);
				vec3 ioffset = ipart + offset;
				uint8_t blockDistance = get_instance_data(upos.x + dx, upos.y, upos.z + dz);
				if(blockDistance & 0x01) {
					vec3 blockCenter((int)(upos.x + dx) + 0.5f, (int)upos.y + 0.5f, (int)(upos.z + dz) + 0.5f);
					float dist = length(blockCenter - upos);
					if(collisionFlag == 0 && dist < 0.958)
					{

						for(int i=0; i<12; i++) {
							vec3 t0 = vertices[i*3].vertex + ioffset;
							vec3 t1 = vertices[(i*3)+1].vertex + ioffset;
							vec3 t2 = vertices[(i*3)+2].vertex + ioffset;
							float t = 0;
							bool isect = rayTriangleIntersect(
								upos, upos - orig, t0, t1, t2, t);
							if(isect == true)
							{
								printf("Collision with face %i\n", i);
							}
						}

						collisionFlag = 1;
						upos = orig;
					}
				}
			}
		}

		GLint viewLoc = glGetUniformLocation(prog, "view");
		vec3 up = vec3(0.0f, 1.0f, 0.0f);
		mat4 viewMatrix = lookAt(upos, upos + dir, up);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(viewMatrix));

		GLint modelLoc = glGetUniformLocation(prog, "model");
		mat4 modelMatrix = mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(modelMatrix));

		//glDrawArrays(GL_TRIANGLES, 0, NUM_BLOCK_VERTICES);
		glBindTexture(GL_TEXTURE_2D, texid);
		glBindVertexArray(vao);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 
			NUM_BLOCK_VERTICES, (GLsizei)NumberOfInstances);
		glBindVertexArray(0);
		GLCHKERR("draw instances");

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("IMGUI Window");
		double thisTime = glfwGetTime();
		ImGui::Text("%.1f fps", 1.0 / (thisTime - lastTime));
		ImGui::Text("CFLAG %i", collisionFlag);
		ImGui::End();

		lastTime = thisTime;
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	printf("Terminate.\n");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	cleanup();
	glfwTerminate();
	return 0;
}

// is upos in block
//   ws prev pos up/down/left/right/front/back
//   determine face moved through
//   get normal of face
//   got starting point
//   got goal
//   intersect ray with face triangle to find collision point
//   vGC = vector from goal to collision point
//   n = face normal
//   l = vGC dot n
//   sx = Gx + (l * nx)
//   sy = Gy + (l * ny)

float kEpsilon = 0.001f;

void slide(vec3& goal, vec3& collision, vec3& normal, vec3& slideTo)
{
	vec3 gc = collision - goal;
	float l = dot(gc, normal);
	slideTo.x = goal.x + (l * normal.x);
	slideTo.y = goal.y + (l * normal.y);
	slideTo.z = goal.z + (l * normal.z);
}

// orig origin
// dir direction
// v0, v1, v2 triangle vertices
// bool rayTriangleIntersect(
// 	const vec3& orig, const vec3& dir,
// 	const vec3& v0, const vec3& v1, const vec3& v2,
// 	float& t, float& u, float& v)
// {
// 	vec3 v0v1 = v1 - v0;
// 	vec3 v0v2 = v2 - v0;
// 	vec3 pvec = cross(dir, v0v2); // dir.crossProduct(v0v2);
// 	float det = dot(v0v1, pvec); // v0v1.dotProduct(pvec);

// #ifdef CULLING 
// 	// if the determinant is negative the triangle is backfacing
// 	// if the determinant is close to 0, the ray misses the triangle
// 	if (det < kEpsilon) return false;
// #else 
// 	// ray and triangle are parallel if det is close to 0
// 	if (fabs(det) < kEpsilon) return false;
// #endif 

// 	float invDet = 1 / det;

// 	vec3 tvec = orig - v0;
// 	u = dot(tvec, pvec) * invDet; // tvec.dotProduct(pvec) * invDet;
// 	if (u < 0 || u > 1) return false;

// 	vec3 qvec = cross(tvec, v0v1); // tvec.crossProduct(v0v1);
// 	v = dot(dir, qvec) * invDet; // dir.dotProduct(qvec) * invDet;
// 	if (v < 0 || u + v > 1) return false;

// 	t = dot(v0v2, qvec) * invDet; // v0v2.dotProduct(qvec)* invDet;

// 	return true;
// }

bool rayTriangleIntersect( 
    const vec3 &orig, const vec3 &dir, 
    const vec3 &v0, const vec3 &v1, const vec3 &v2, 
    float &t) 
{ 
    // compute plane's normal
    vec3 v0v1 = v1 - v0; 
    vec3 v0v2 = v2 - v0; 
    // no need to normalize
    vec3 N = cross(v0v1, v0v2); // v0v1.crossProduct(v0v2); // N 
    float area2 = N.length(); 
 
    // Step 1: finding P
 
    // check if ray and plane are parallel ?
    float NdotRayDirection = dot(N, dir); // N.dotProduct(dir); 
    if (fabs(NdotRayDirection) < kEpsilon) // almost 0 
        return false; // they are parallel so they don't intersect ! 
 
    // compute d parameter using equation 2
    float d = dot(N, v0); // N.dotProduct(v0); 
 
    // compute t (equation 3)
    //t = (N.dotProduct(orig) + d) / NdotRayDirection; 
	t = (dot(N, orig) + d) / NdotRayDirection; 
    // check if the triangle is in behind the ray
    if (t < 0) return false; // the triangle is behind 
 
    // compute the intersection point using equation 1
    vec3 P = orig + t * dir; 
 
    // Step 2: inside-outside test
    vec3 C; // vector perpendicular to triangle's plane 
 
    // edge 0
    vec3 edge0 = v1 - v0; 
    vec3 vp0 = P - v0; 
    C = cross(edge0, vp0); // edge0.crossProduct(vp0); 
    //if (N.dotProduct(C) < 0) return false; // P is on the right side 
	if (dot(N, C) < 0) return false; // P is on the right side 
 
    // edge 1
    vec3 edge1 = v2 - v1; 
    vec3 vp1 = P - v1; 
    C = cross(edge1, vp1); // edge1.crossProduct(vp1); 
    //if (N.dotProduct(C) < 0)  return false; // P is on the right side 
	if (dot(N, C) < 0)  return false; // P is on the right side 
 
    // edge 2
    vec3 edge2 = v0 - v2; 
    vec3 vp2 = P - v2; 
    C = cross(edge2, vp2); // edge2.crossProduct(vp2); 
    //if (N.dotProduct(C) < 0) return false; // P is on the right side; 
	if (dot(N, C) < 0) return false; // P is on the right side; 
 
    return true; // this ray hits the triangle 
} 
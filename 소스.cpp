#include <iostream>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <vector>

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/glm.hpp"
#include "GL/ext.hpp"
#include "GL/gtc/matrix_transform.hpp"
#include "GL/freeglut_ext.h"

using namespace std;
using namespace glm;

float r = 0.0f;
float g = 0.0f;
float b = 0.0f;
float a = 0.f;
int width = 800;
int height = 800;

int tick = 110;
int signX = 1;
int signY = 1;
int drawingChange = 1;
int cnt = 0;
int FragShape = 0;

bool fillMode = false;
bool path = true;
bool cutterOn = true;
bool brk = false;

float moveX = 0.05f;
float moveY = 0.025f;
float frag1Y;
float storeX = 0.f;
float storeY = 0.f;
float goalX = 0.f;
float goalY = 0.f;
float mouseDownX;
float mouseDownY;
float mouseUpX;
float mouseUpY;
float collideX1 = 2.f;
float collideY1 = 2.f;
float collideX2 = 2.f;
float collideY2 = 2.f;

GLuint programID = 0;

struct TriType
{
	GLuint VAO;
	GLuint VBO;
};

struct RectType
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
};

struct RoadType
{
	GLuint VAO;
	GLuint VBO;
};

struct CutterType
{
	GLuint VAO;
	GLuint VBO;
};

struct TriFragType1
{
	GLuint VAO;
	GLuint VBO;
};

struct TriFragType2
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
};

struct RectFragType1
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
};

struct RectFragType2
{
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
};

TriType triType;
RectType rectType;
RoadType roadType;
CutterType cutterType;
TriFragType1 triFragType1;//smaller
TriFragType2 triFragType2;//bigger
RectFragType1 rectFragType1;//smaller
RectFragType2 rectFragType2;//bigger

mat4 myMat = mat4(1.f);
mat4 revMat = mat4(1.f);
mat4 transMat = mat4(1.f);
mat4 rotMat = mat4(1.f);

mat4 frag1Mat = mat4(1.f);
mat4 frag2Mat = mat4(1.f);

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	string VertexShaderCode;
	ifstream VertexShaderStream(vertex_file_path, ios::in);
	if (VertexShaderStream.is_open()) 
	{
		stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("파일 %s 를 읽을 수 없음\n", vertex_file_path);
		return 0;
	}

	string FragmentShaderCode;
	ifstream FragmentShaderStream(fragment_file_path, ios::in);
	if (FragmentShaderStream.is_open()) 
	{
		stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}
	else 
	{
		printf("파일 %s 를 읽을 수 없음\n", fragment_file_path);
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) 
	{
		vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) 
	{
		vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) 
	{
		vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

GLvoid onReshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}

float rectFragData1[] =
{
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
};

float rectFragData2[] =
{
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
	2.f, 2.f, 0.f, 1.f, 0.f, 0.0f,
};

float triFragData1[] =
{
	2.f, 2.f, 0.f, 1.f, 1.f, 0.5f,//0
	2.f, 2.f, 0.f, 1.f, 1.f, 0.5f,//1
	2.f, 2.f, 0.f, 1.f, 1.f, 0.5f,//2
};

float triFragData2[] =
{
	2.f, 2.f, 0.f, 1.f, 1.f, 0.5f,//0
	2.f, 2.f, 0.f, 1.f, 1.f, 0.5f,//1
	2.f, 2.f, 0.f, 1.f, 1.f, 0.5f,//2
	2.f, 2.f, 0.f, 1.f, 1.f, 0.5f,//3
};

int triFragIndex[6]{ 0 };

int rectFragIndex1[9]{ 0 };

int rectFragIndex2[9]{ 0 };

float roadData[12] = 
{
	2.f, 2.f, 0.f, 1.f, 1.f, 1.f,
	2.f, 2.f, 0.f, 1.f, 1.f, 1.f,
};

float cutterData[12] =
{
	2.f, 2.f, 0.f, 1.f, 0.f, 0.f,
	2.f, 2.f, 0.f, 0.f, 0.f, 1.f,
};

void createCutter()
{
	glGenVertexArrays(1, &cutterType.VAO);
	glGenBuffers(1, &cutterType.VBO);

	glBindVertexArray(cutterType.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, cutterType.VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cutterData), cutterData, GL_STATIC_DRAW);

	GLuint vPos1 = glGetAttribLocation(programID, "vPos1");
	glVertexAttribPointer(vPos1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
	glEnableVertexAttribArray(vPos1);

	GLuint vColor1 = glGetAttribLocation(programID, "vColor1");
	glVertexAttribPointer(vColor1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vColor1);
}

void makeRoadData()
{
	drawingChange++;

	float startTmp = (rand() / (float)RAND_MAX) * 1000;
	int tmp = startTmp;
	float startValue = startTmp - tmp;

	int rightOrLeft = rand() % 2 + 1;
	int upOrDown = rand() % 2 + 1;

	if (rightOrLeft == 1)
	{
		if (upOrDown == 1)
		{
			storeY = startValue;
			signY = -1;
		}
		else
		{
			storeY = -startValue;
			signY = 1;
		}
		storeX = -1.3f;
		signX = 1;
	}
	else
	{
		if (upOrDown == 1)
		{
			storeY = startValue;
			signY = -1;
		}
		else
		{
			storeY = -startValue;
			signY = 1;
		}
		storeX = 1.3f;
		signX = -1;
	}

	goalX = storeX;
	goalY = storeY;
	while (true)
	{
		if ((signX == 1 && goalX >= 1.f + 0.15f) || (signX == -1 && goalX <= -1.f - 0.15f) ||
			(signY == 1 && goalY >= 1.f + 0.15f) || (signY == -1 && goalY <= -1.f - 0.15f))
		{
			break;
		}
		goalX += (signX * 0.05f);
		goalY += (signY * 0.025f);
	}

	roadData[0] = storeX;
	roadData[1] = storeY;
	roadData[6] = goalX;
	roadData[7] = goalY;
}

float triData[] =
{
	0.f, 0.15f, 0.f, 1.f, 1.f, 0.5f,
	0.15f, -0.15f, 0.f, 1.f, 1.f, 0.5f,
	-0.15f, -0.15f, 0.f, 1.f, 1.f, 0.5f,
};

float rectData[] =
{
	-0.15f, 0.15f, 0.f, 1.f, 0.f, 0.f,//0
	0.15f, 0.15f, 0.f, 1.f, 0.f, 0.f,//1
	0.15f, -0.15f, 0.f, 1.f, 0.f, 0.f,//2
	-0.15f, -0.15f, 0.f, 1.f, 0.f, 0.f,//3
};

int rectIndex[] =
{
	0,2,1,
	0,3,2,
};

void createRect()
{
	transMat = translate(transMat, vec3(2.f, 2.f, 0.f));

	glGenVertexArrays(1, &rectType.VAO);
	glGenBuffers(1, &rectType.VBO);
	glGenBuffers(1, &rectType.EBO);

	glBindVertexArray(rectType.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectType.VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(rectData), rectData, GL_STATIC_DRAW);

	GLuint vPos1 = glGetAttribLocation(programID, "vPos1");
	glVertexAttribPointer(vPos1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(vPos1);

	GLuint vColor1 = glGetAttribLocation(programID, "vColor1");
	glVertexAttribPointer(vColor1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vColor1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectType.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndex), rectIndex, GL_STATIC_DRAW);
}

void createTriFrag2()
{
	glGenVertexArrays(1, &triFragType2.VAO);
	glGenBuffers(1, &triFragType2.VBO);
	glGenBuffers(1, &triFragType2.EBO);

	glBindVertexArray(triFragType2.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, triFragType2.VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(triFragData2), triFragData2, GL_STATIC_DRAW);

	GLuint vPos1 = glGetAttribLocation(programID, "vPos1");
	glVertexAttribPointer(vPos1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(vPos1);

	GLuint vColor1 = glGetAttribLocation(programID, "vColor1");
	glVertexAttribPointer(vColor1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vColor1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triFragType2.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triFragIndex), triFragIndex, GL_STATIC_DRAW);
}

void createTriFrag1()
{
	glGenVertexArrays(1, &triFragType1.VAO);
	glGenBuffers(1, &triFragType1.VBO);

	glBindVertexArray(triFragType1.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, triFragType1.VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(triFragData1), triFragData1, GL_STATIC_DRAW);

	GLuint vPos1 = glGetAttribLocation(programID, "vPos1");
	glVertexAttribPointer(vPos1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
	glEnableVertexAttribArray(vPos1);

	GLuint vColor1 = glGetAttribLocation(programID, "vColor1");
	glVertexAttribPointer(vColor1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vColor1);
}

void createRectFrag1()
{
	glGenVertexArrays(1, &rectFragType1.VAO);
	glGenBuffers(1, &rectFragType1.VBO);
	glGenBuffers(1, &rectFragType1.EBO);

	glBindVertexArray(rectFragType1.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectFragType1.VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(rectFragData1), rectFragData1, GL_STATIC_DRAW);

	GLuint vPos1 = glGetAttribLocation(programID, "vPos1");
	glVertexAttribPointer(vPos1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(vPos1);

	GLuint vColor1 = glGetAttribLocation(programID, "vColor1");
	glVertexAttribPointer(vColor1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vColor1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectFragType1.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectFragIndex1), rectFragIndex1, GL_STATIC_DRAW);
}

void createRectFrag2()
{
	glGenVertexArrays(1, &rectFragType2.VAO);
	glGenBuffers(1, &rectFragType2.VBO);
	glGenBuffers(1, &rectFragType2.EBO);

	glBindVertexArray(rectFragType2.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectFragType2.VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(rectFragData2), rectFragData2, GL_STATIC_DRAW);

	GLuint vPos1 = glGetAttribLocation(programID, "vPos1");
	glVertexAttribPointer(vPos1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(vPos1);

	GLuint vColor1 = glGetAttribLocation(programID, "vColor1");
	glVertexAttribPointer(vColor1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vColor1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectFragType2.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectFragIndex2), rectFragIndex2, GL_STATIC_DRAW);
}

void createTri()
{
	transMat = translate(transMat, vec3(2.f, 2.f, 0.f));

	glGenVertexArrays(1, &triType.VAO);
	glGenBuffers(1, &triType.VBO);

	glBindVertexArray(triType.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, triType.VBO);
		
	glBufferData(GL_ARRAY_BUFFER, sizeof(triData), triData, GL_STATIC_DRAW);
		
	GLuint vPos1 = glGetAttribLocation(programID, "vPos1");
	glVertexAttribPointer(vPos1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
	glEnableVertexAttribArray(vPos1);
		
	GLuint vColor1 = glGetAttribLocation(programID, "vColor1");
	glVertexAttribPointer(vColor1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vColor1);
}

void createRoad()
{
	glGenVertexArrays(1, &roadType.VAO);
	glGenBuffers(1, &roadType.VBO);

	glBindVertexArray(roadType.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, roadType.VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(roadData), roadData, GL_STATIC_DRAW);

	GLuint vPos1 = glGetAttribLocation(programID, "vPos1");
	glVertexAttribPointer(vPos1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
	glEnableVertexAttribArray(vPos1);

	GLuint vColor1 = glGetAttribLocation(programID, "vColor1");
	glVertexAttribPointer(vColor1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(vColor1);
}

void convertDeviceXY2OpenglXY(int x, int y, float* nx, float* ny)
{
	int w = width;
	int h = height;
	*nx = (float)((x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0)));
	*ny = -(float)((y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0)));
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		cutterOn = false;
		convertDeviceXY2OpenglXY(x, y, &mouseDownX, &mouseDownY);
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		cnt = 0;
		cutterOn = true;
		convertDeviceXY2OpenglXY(x, y, &mouseUpX, &mouseUpY);
		cutterData[0] = mouseDownX;
		cutterData[1] = mouseDownY;
		cutterData[6] = mouseUpX;
		cutterData[7] = mouseUpY;
	}
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '=':
		if (!(moveX >= 0.2f || moveY >= 0.1f))
		{
			moveX += 0.01f;
			moveY += 0.005f;
		}
		break;
	case '-':
		if (!(moveX <= 0.05f || moveY <= 0.025f))
		{
			moveX += -0.01f;
			moveY += -0.005f;
		}
		break;
	case 'p':
		if (path)
		{
			path = false;
		}
		else
		{
			path = true;
		}
		break;

	case 'f':
		if (!fillMode)
		{
			glPolygonMode(GLenum(GL_FRONT_AND_BACK), GLenum(GL_LINE));
			fillMode = true;
		}
		else
		{
			glPolygonMode(GLenum(GL_FRONT_AND_BACK), GLenum(GL_FILL));
			fillMode = false;
		}
		break;

	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

bool isCollide(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
	float p1 = 0.f;
	float p2 = 0.f;
	float denom = (cutterData[7] - cutterData[1]) * (x2 - x1) - (cutterData[6] - cutterData[0]) * (y2 - y1);
	float nume1 = (cutterData[6] - cutterData[0]) * (y1 - cutterData[1]) - (cutterData[7] - cutterData[1]) * (x1 - cutterData[0]);
	float nume2 = (x2 - x1) * (y1 - cutterData[1]) - (y2 - y1) * (x1 - cutterData[0]);
	p1 = nume1 / denom;
	p2 = nume2 / denom;

	if (denom == 0.f)
	{
		collideX1 = 2.f;
		collideY1 = 2.f;

		return false;
	}
	if (p1<0.0 || p1>1.0 || p2<0.0 || p2>1.0)
	{
		collideX1 = 2.f;
		collideY1 = 2.f;

		return false;
	}

	collideX1 = x1 + p1 * (x2 - x1);
	collideY1 = y1 + p1 * (y2 - y1);

	denom = (cutterData[7] - cutterData[1]) * (x4 - x3) - (cutterData[6] - cutterData[0]) * (y4 - y3);
	nume1 = (cutterData[6] - cutterData[0]) * (y3 - cutterData[1]) - (cutterData[7] - cutterData[1]) * (x3 - cutterData[0]);
	nume2 = (x4 - x3) * (y3 - cutterData[1]) - (y4 - y3) * (x3 - cutterData[0]);
	p1 = nume1 / denom;
	p2 = nume2 / denom;

	if (denom == 0.f)
	{
		collideX2 = 2.f;
		collideY2 = 2.f;

		return false;
	}
	if (p1<0.0 || p1>1.0 || p2<0.0 || p2>1.0)
	{
		collideX2 = 2.f;
		collideY2 = 2.f;

		return false;
	}

	collideX2 = x3 + p1 * (x4 - x3);
	collideY2 = y3 + p1 * (y4 - y3);

	return true;
}

void makeTriFragDataAndIndex_Shape1()
{
	triFragData1[0] = collideX2; triFragData1[1] = collideY2;
	triFragData1[6] = collideX1; triFragData1[7] = storeY-0.15f;
	triFragData1[12] = storeX + 0.15f; triFragData1[13] = storeY - 0.15f;

	triFragData2[0] = storeX; triFragData2[1] = storeY + 0.15f;//0
	triFragData2[6] = storeX-0.15f; triFragData2[7] = storeY - 0.15f;//1
	triFragData2[12] = collideX1; triFragData2[13] = storeY - 0.15f;//2
	triFragData2[18] = collideX2; triFragData2[19] = collideY2;//3

	triFragIndex[0] = 0; triFragIndex[1] = 2;
	triFragIndex[2] = 3; triFragIndex[3] = 0;
	triFragIndex[4] = 1; triFragIndex[5] = 2;
}

void makeTriFragDataAndIndex_Shape2()
{
	triFragData1[0] = storeX; triFragData1[1] = storeY + 0.15f;
	triFragData1[6] = collideX1; triFragData1[7] = collideY1;
	triFragData1[12] = collideX2; triFragData1[13] = collideY2;

	triFragData2[0] = collideX1; triFragData2[1] = collideY1;//0
	triFragData2[6] = storeX - 0.15f; triFragData2[7] = storeY - 0.15f;//1
	triFragData2[12] = storeX + 0.15f; triFragData2[13] = storeY - 0.15f;//2
	triFragData2[18] = collideX2; triFragData2[19] = collideY2;//3

	triFragIndex[0] = 0; triFragIndex[1] = 1;
	triFragIndex[2] = 3; triFragIndex[3] = 3;
	triFragIndex[4] = 1; triFragIndex[5] = 2;
}

void makeTriFragDataAndIndex_Shape3()
{
	triFragData1[0] = collideX2; triFragData1[1] = collideY2;
	triFragData1[6] = storeX - 0.15f; triFragData1[7] = storeY-0.15f;
	triFragData1[12] = collideX1; triFragData1[13] = storeY - 0.15f;

	triFragData2[0] = storeX; triFragData2[1] = storeY+0.15f;//0
	triFragData2[6] = collideX2; triFragData2[7] = collideY2;//1
	triFragData2[12] = collideX1; triFragData2[13] = storeY - 0.15f;//2
	triFragData2[18] = storeX+0.15f; triFragData2[19] = storeY-0.15f;//3

	triFragIndex[0] = 0; triFragIndex[1] = 1;
	triFragIndex[2] = 2; triFragIndex[3] = 0;
	triFragIndex[4] = 2; triFragIndex[5] = 3;
}

void makeRectFragDataAndIndex_shape1()
{
	rectFragData1[0] = storeX-0.15f; rectFragData1[1] = storeY+0.15f;//0
	rectFragData1[6] = storeX - 0.15f; rectFragData1[7] = collideY1;//1
	rectFragData1[12] = collideX2; rectFragData1[13] = storeY + 0.15f;//2
	rectFragData1[18] = collideX2; rectFragData1[19] = storeY + 0.15f;//none
	rectFragData1[24] = collideX2; rectFragData1[25] = storeY + 0.15f;//none

	rectFragData2[0] = collideX2; rectFragData2[1] = storeY + 0.15f;//0
	rectFragData2[6] = storeX - 0.15f; rectFragData2[7] = collideY1;//1
	rectFragData2[12] = storeX - 0.15f; rectFragData2[13] = storeY - 0.15f;//2
	rectFragData2[18] = storeX + 0.15f; rectFragData2[19] = storeY - 0.15f;//3
	rectFragData2[24] = storeX + 0.15f; rectFragData2[25] = storeY + 0.15f;//4

	rectFragIndex1[0] = 0; rectFragIndex1[1] = 1; rectFragIndex1[2] = 2; 
	rectFragIndex1[3] = 0; rectFragIndex1[4] = 1; rectFragIndex1[5] = 2;
	rectFragIndex1[6] = 0; rectFragIndex1[7] = 1; rectFragIndex1[8] = 2;
	
	rectFragIndex2[0] = 4; rectFragIndex2[1] = 0; rectFragIndex2[2] = 3;
	rectFragIndex2[3] = 0; rectFragIndex2[4] = 1; rectFragIndex2[5] = 3;
	rectFragIndex2[6] = 1; rectFragIndex2[7] = 2; rectFragIndex2[8] = 3;
}

void makeRectFragDataAndIndex_shape2()
{
	rectFragData1[0] = storeX + 0.15f; rectFragData1[1] = storeY + 0.15f;//0
	rectFragData1[6] = collideX1; rectFragData1[7] = storeY + 0.15f;//1
	rectFragData1[12] = storeX + 0.15f; rectFragData1[13] = collideY2;//2
	rectFragData1[18] = storeX + 0.15f; rectFragData1[19] = collideY2;//none
	rectFragData1[24] = storeX + 0.15f; rectFragData1[25] = collideY2;//none

	rectFragData2[0] = collideX1; rectFragData2[1] = storeY + 0.15f;//0
	rectFragData2[6] = storeX - 0.15f; rectFragData2[7] = storeY+0.15f;//1
	rectFragData2[12] = storeX - 0.15f; rectFragData2[13] = storeY - 0.15f;//2
	rectFragData2[18] = storeX + 0.15f; rectFragData2[19] = storeY - 0.15f;//3
	rectFragData2[24] = storeX + 0.15f; rectFragData2[25] = collideY2;//4

	rectFragIndex1[0] = 0; rectFragIndex1[1] = 1; rectFragIndex1[2] = 2;
	rectFragIndex1[3] = 0; rectFragIndex1[4] = 1; rectFragIndex1[5] = 2;
	rectFragIndex1[6] = 0; rectFragIndex1[7] = 1; rectFragIndex1[8] = 2;

	rectFragIndex2[0] = 0; rectFragIndex2[1] = 1; rectFragIndex2[2] = 2;
	rectFragIndex2[3] = 0; rectFragIndex2[4] = 2; rectFragIndex2[5] = 4;
	rectFragIndex2[6] = 4; rectFragIndex2[7] = 2; rectFragIndex2[8] = 3;
}

void makeRectFragDataAndIndex_shape3()
{
	rectFragData1[0] = storeX-0.15f; rectFragData1[1] = collideY1;//0
	rectFragData1[6] = storeX - 0.15f; rectFragData1[7] = storeY-0.15f;//1
	rectFragData1[12] = collideX2; rectFragData1[13] = storeY - 0.15f;//2
	rectFragData1[18] = collideX2; rectFragData1[19] = storeY - 0.15f;//none
	rectFragData1[24] = collideX2; rectFragData1[25] = storeY - 0.15f;//none

	rectFragData2[0] = storeX-0.15f; rectFragData2[1] = storeY + 0.15f;//0
	rectFragData2[6] = storeX - 0.15f; rectFragData2[7] = collideY1;//1
	rectFragData2[12] = collideX2; rectFragData2[13] = storeY - 0.15f;//2
	rectFragData2[18] = storeX + 0.15f; rectFragData2[19] = storeY - 0.15f;//3
	rectFragData2[24] = storeX + 0.15f; rectFragData2[25] = storeY + 0.15f;//4

	rectFragIndex1[0] = 0; rectFragIndex1[1] = 1; rectFragIndex1[2] = 2;
	rectFragIndex1[3] = 0; rectFragIndex1[4] = 1; rectFragIndex1[5] = 2;
	rectFragIndex1[6] = 0; rectFragIndex1[7] = 1; rectFragIndex1[8] = 2;

	rectFragIndex2[0] = 4; rectFragIndex2[1] = 0; rectFragIndex2[2] = 1;
	rectFragIndex2[3] = 4; rectFragIndex2[4] = 1; rectFragIndex2[5] = 2;
	rectFragIndex2[6] = 4; rectFragIndex2[7] = 2; rectFragIndex2[8] = 3;
}

void makeRectFragDataAndIndex_shape4()
{
	rectFragData1[0] = storeX + 0.15f; rectFragData1[1] = collideY2;//0
	rectFragData1[6] = collideX1; rectFragData1[7] = storeY-0.15f;//1
	rectFragData1[12] = storeX + 0.15f; rectFragData1[13] = storeY - 0.15f;//2
	rectFragData1[18] = storeX + 0.15f; rectFragData1[19] = storeY - 0.15f;//none
	rectFragData1[24] = storeX + 0.15f; rectFragData1[25] = storeY - 0.15f;//none

	rectFragData2[0] = storeX - 0.15f; rectFragData2[1] = storeY + 0.15f;//0
	rectFragData2[6] = storeX - 0.15f; rectFragData2[7] = storeY - 0.15f;//1
	rectFragData2[12] = collideX1; rectFragData2[13] = storeY - 0.15f;//2
	rectFragData2[18] = storeX + 0.15f; rectFragData2[19] = collideY2;//3
	rectFragData2[24] = storeX + 0.15f; rectFragData2[25] = storeY + 0.15f;//4

	rectFragIndex1[0] = 0; rectFragIndex1[1] = 1; rectFragIndex1[2] = 2;
	rectFragIndex1[3] = 0; rectFragIndex1[4] = 1; rectFragIndex1[5] = 2;
	rectFragIndex1[6] = 0; rectFragIndex1[7] = 1; rectFragIndex1[8] = 2;

	rectFragIndex2[0] = 0; rectFragIndex2[1] = 1; rectFragIndex2[2] = 2;
	rectFragIndex2[3] = 0; rectFragIndex2[4] = 2; rectFragIndex2[5] = 3;
	rectFragIndex2[6] = 0; rectFragIndex2[7] = 3; rectFragIndex2[8] = 4;
}

void makeRectFragDataAndIndex_shape5()
{
	rectFragData1[0] = storeX - 0.15f; rectFragData1[1] = storeY+0.15f;//0
	rectFragData1[6] = collideX1; rectFragData1[7] = collideY1;//1
	rectFragData1[12] = collideX2; rectFragData1[13] = collideY2;//2
	rectFragData1[18] = storeX + 0.15f; rectFragData1[19] = storeY+0.15f;//3
	rectFragData1[24] = storeX + 0.15f; rectFragData1[25] = storeY + 0.15f;//none

	rectFragData2[0] = collideX1; rectFragData2[1] = collideY1;//0
	rectFragData2[6] = storeX - 0.15f; rectFragData2[7] = storeY - 0.15f;//1
	rectFragData2[12] = storeX+0.15f; rectFragData2[13] = storeY-0.15f;//2
	rectFragData2[18] = collideX2; rectFragData2[19] = collideY2;//3
	rectFragData2[24] = collideX2; rectFragData2[25] = collideY2;//none

	rectFragIndex1[0] = 3; rectFragIndex1[1] = 0; rectFragIndex1[2] = 1;
	rectFragIndex1[3] = 3; rectFragIndex1[4] = 1; rectFragIndex1[5] = 2;
	rectFragIndex1[6] = 3; rectFragIndex1[7] = 1; rectFragIndex1[8] = 2;

	rectFragIndex2[0] = 3; rectFragIndex2[1] = 0; rectFragIndex2[2] = 1;
	rectFragIndex2[3] = 3; rectFragIndex2[4] = 1; rectFragIndex2[5] = 2;
	rectFragIndex2[6] = 3; rectFragIndex2[7] = 1; rectFragIndex2[8] = 2;
}

void makeRectFragDataAndIndex_shape6()
{
	rectFragData1[0] = storeX - 0.15f; rectFragData1[1] = storeY+0.15f;//0
	rectFragData1[6] = storeX-0.15f; rectFragData1[7] = storeY - 0.15f;//1
	rectFragData1[12] = collideX1; rectFragData1[13] = collideY1;//2
	rectFragData1[18] = collideX2; rectFragData1[19] = collideY2;//3
	rectFragData1[24] = collideX2; rectFragData1[25] = collideY2;//none

	rectFragData2[0] = collideX2; rectFragData2[1] = collideY2;//0
	rectFragData2[6] = collideX1; rectFragData2[7] = collideY1;//1
	rectFragData2[12] = storeX+0.15f; rectFragData2[13] = storeY - 0.15f;//2
	rectFragData2[18] = storeX + 0.15f; rectFragData2[19] = storeY+0.15f;//3
	rectFragData2[24] = storeX + 0.15f; rectFragData2[25] = storeY + 0.15f;//none

	rectFragIndex1[0] = 0; rectFragIndex1[1] = 1; rectFragIndex1[2] = 2;
	rectFragIndex1[3] = 0; rectFragIndex1[4] = 2; rectFragIndex1[5] = 3;
	rectFragIndex1[6] = 0; rectFragIndex1[7] = 2; rectFragIndex1[8] = 3;

	rectFragIndex2[0] = 0; rectFragIndex2[1] = 1; rectFragIndex2[2] = 2;
	rectFragIndex2[3] = 0; rectFragIndex2[4] = 2; rectFragIndex2[5] = 3;
	rectFragIndex2[6] = 0; rectFragIndex2[7] = 2; rectFragIndex2[8] = 3;
}

void timer(int value)
{
	if (!brk)
	{
		if ((drawingChange % 2 == 1))//삼각형
		{
			if (isCollide(storeX - 0.15f, storeY - 0.15f, storeX + 0.15f, storeY - 0.15f, storeX, storeY + 0.15f, storeX + 0.15f, storeY - 0.15f))
			{
					FragShape = 1;
					makeTriFragDataAndIndex_Shape1();
			}
			else if (isCollide(storeX - 0.15f, storeY - 0.15f, storeX, storeY + 0.15f, storeX, storeY + 0.15f, storeX + 0.15f, storeY - 0.15f))
			{
					FragShape = 2;
					makeTriFragDataAndIndex_Shape2();
			}
			else if (isCollide(storeX - 0.15f, storeY - 0.15f, storeX + 0.15f, storeY - 0.15f, storeX - 0.15f, storeY - 0.15f, storeX, storeY + 0.15f))
			{
					FragShape = 3;
					makeTriFragDataAndIndex_Shape3();
			}
		}
		else//사각형
		{
			if (isCollide(storeX - 0.15f, storeY - 0.15f, storeX - 0.15f, storeY + 0.15f, storeX - 0.15f, storeY + 0.15f, storeX + 0.15f, storeY + 0.15f))
			{
				FragShape = 1;
				makeRectFragDataAndIndex_shape1();
			}
			else if (isCollide(storeX - 0.15f, storeY + 0.15f, storeX + 0.15f, storeY + 0.15f, storeX + 0.15f, storeY + 0.15f, storeX + 0.15f, storeY - 0.15f))
			{
				FragShape = 2;
				makeRectFragDataAndIndex_shape2();
			}
			else if (isCollide(storeX - 0.15f, storeY + 0.15f, storeX - 0.15f, storeY - 0.15f, storeX - 0.15f, storeY - 0.15f, storeX + 0.15f, storeY - 0.15f))
			{
				FragShape = 3;
				makeRectFragDataAndIndex_shape3();
			}
			else if (isCollide(storeX - 0.15f, storeY - 0.15f, storeX + 0.15f, storeY - 0.15f, storeX + 0.15f, storeY - 0.15f, storeX + 0.15f, storeY + 0.15f))
			{
				FragShape = 4;
				makeRectFragDataAndIndex_shape4();
			}
			else if (isCollide(storeX - 0.15f, storeY - 0.15f, storeX - 0.15f, storeY + 0.15f, storeX + 0.15f, storeY - 0.15f, storeX + 0.15f, storeY + 0.15f))
			{
				FragShape = 5;
				makeRectFragDataAndIndex_shape5();
			}
			else if (isCollide(storeX - 0.15f, storeY - 0.15f, storeX + 0.15f, storeY - 0.15f, storeX - 0.15f, storeY + 0.15f, storeX + 0.15f, storeY + 0.15f))
			{
				FragShape = 6;
				makeRectFragDataAndIndex_shape6();
			}
		}
		if (FragShape >= 1)
		{
			brk = true;
			cutterData[0] = cutterData[1] = cutterData[6] = cutterData[7] = 2.f;
			collideX1 = collideX2 = collideY1 = collideY2 = 2.f;
			frag1Y = storeY + 0.15f;
		}
	}
	if (brk)
	{
		frag1Mat = translate(frag1Mat, vec3(0.f, -0.1f, 0.f));
		frag2Mat = translate(frag2Mat, vec3(0.f, -0.2f, 0.f));
		frag1Y += -0.1f;
		if (frag1Y <= -1.f)
		{
			brk = false;
			FragShape = 0;
			frag1Mat = mat4(1.f);
			frag2Mat = mat4(1.f);
			makeRoadData();
		}
	}

	if (!brk)
	{
		if ((signX == 1 && storeX >= 1.f + 0.15f) || (signX == -1 && storeX <= -1.f - 0.15f) ||
			(signY == 1 && storeY >= 1.f + 0.15f) || (signY == -1 && storeY <= -1.f - 0.15f))
		{
			makeRoadData();
		}
		transMat = mat4(1.f);
		storeX += (signX * moveX);
		storeY += (signY * moveY);
		transMat = translate(transMat, vec3(storeX, storeY, 0.f));
	}

	if (cutterOn)
	{
		cnt++;
	}
	if (cnt == 5)
	{
		cnt = 0;
		cutterOn = false;
		cutterData[0] = cutterData[1] = cutterData[6] = cutterData[7] = -2.f;
	}
	
	glutPostRedisplay();
	glutTimerFunc(tick, timer, 1);
}

GLvoid onDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(programID);
	unsigned int modelLocation = glGetUniformLocation(programID, "transform");
	static const mat4 identity(1.f);
	myMat = revMat * transMat * rotMat;

	if (!brk)
	{
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(identity));
		if (path)
		{
			glBindVertexArray(roadType.VAO);
			glBindBuffer(GL_ARRAY_BUFFER, roadType.VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(roadData), roadData, GL_STATIC_DRAW);
			glDrawArrays(GL_LINE_STRIP, 0, 2);
		}

		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(myMat));
		if (drawingChange % 2 == 1)
		{
			glBindVertexArray(triType.VAO);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
		else
		{
			glBindVertexArray(rectType.VAO);
			glDrawElements(GL_TRIANGLES, size(rectIndex), GL_UNSIGNED_INT, nullptr);
		}

	}
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(identity));
	if (cutterOn)
	{
		glBindVertexArray(cutterType.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, cutterType.VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cutterData), cutterData, GL_STATIC_DRAW);
		glDrawArrays(GL_LINE_STRIP, 0, 2);
	}
	
	if (brk)
	{
		if (drawingChange % 2 == 1)
		{
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(frag1Mat));

			glBindVertexArray(triFragType1.VAO);
			glBindBuffer(GL_ARRAY_BUFFER, triFragType1.VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(triFragData1), triFragData1, GL_STATIC_DRAW);
			glDrawArrays(GL_TRIANGLES, 0, 3);

			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(frag2Mat));

			glBindVertexArray(triFragType2.VAO);
			glBindBuffer(GL_ARRAY_BUFFER, triFragType2.VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(triFragData2), triFragData2, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triFragType2.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triFragIndex), triFragIndex, GL_STATIC_DRAW);
			glDrawElements(GL_TRIANGLES, size(triFragIndex), GL_UNSIGNED_INT, nullptr);
		}
		else
		{
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(frag1Mat));

			glBindVertexArray(rectFragType1.VAO);
			glBindBuffer(GL_ARRAY_BUFFER, rectFragType1.VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(rectFragData1), rectFragData1, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectFragType1.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectFragIndex1), rectFragIndex1, GL_STATIC_DRAW);
			glDrawElements(GL_TRIANGLES, size(rectFragIndex1), GL_UNSIGNED_INT, nullptr);

			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(frag2Mat));

			glBindVertexArray(rectFragType2.VAO);
			glBindBuffer(GL_ARRAY_BUFFER, rectFragType2.VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(rectFragData2), rectFragData2, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectFragType2.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectFragIndex2), rectFragIndex2, GL_STATIC_DRAW);
			glDrawElements(GL_TRIANGLES, size(rectFragIndex2), GL_UNSIGNED_INT, nullptr);
		}
	}

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	srand((unsigned)time(nullptr));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow("Let's SP");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		cerr << "Unable to initialize GLEW" << endl;
		exit(-1);
	}
	else
	{
		cout << "GLEW Initialized" << endl;
	}

	if (glDebugMessageCallback)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void* userParam) {
			const auto sourceMessage = [source]() 
			{
				switch (source) 
				{
				default:
					return "Unknown";
				case GL_DEBUG_SOURCE_API:
					return "Api";
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
					return "WindowSystem";
				case GL_DEBUG_SOURCE_SHADER_COMPILER:
					return "ShaderCompiler";
				case GL_DEBUG_SOURCE_THIRD_PARTY:
					return "ThirdParty";
				case GL_DEBUG_SOURCE_APPLICATION:
					return "Application";
				case GL_DEBUG_SOURCE_OTHER:
					return "Other";
				}
			}();
			const auto typeMessage = [type]() 
			{
				switch (type)
				{
				default:
					return "Unknown";
				case GL_DEBUG_TYPE_ERROR:
					return "Error";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
					return "DeprecatedBehavior";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
					return "UndefinedBehavior";
				case GL_DEBUG_TYPE_PORTABILITY:
					return "Portability";
				case GL_DEBUG_TYPE_PERFORMANCE:
					return "Performance";
				case GL_DEBUG_TYPE_MARKER:
					return "Marker";
				case GL_DEBUG_TYPE_PUSH_GROUP:
					return "PushGroup";
				case GL_DEBUG_TYPE_POP_GROUP:
					return "PopGroup";
				case GL_DEBUG_TYPE_OTHER:
					return "Other";
				}
			}();
			switch (severity) 
			{
			default:
				return;

			case GL_DEBUG_SEVERITY_LOW:
				cout << "L" << sourceMessage << " : " << typeMessage << " : " << message << endl;
				return;
			case GL_DEBUG_SEVERITY_MEDIUM:
				cout << "M: " << sourceMessage << " : " << typeMessage << " : " << message << endl;
				return;
			case GL_DEBUG_SEVERITY_HIGH:
				cout << "H: " << sourceMessage << " : " << typeMessage << " : " << message << endl;
				return;
			}
			}, nullptr);
	}

	programID = LoadShaders("vs.txt", "fs.txt");
	makeRoadData();
	createRoad();
	createTri();
	createRect();
	createCutter();
	createTriFrag1();
	createTriFrag2();
	createRectFrag1();
	createRectFrag2();
	
	glClearColor(r, g, b, a);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(tick, timer, 1);
	glutReshapeFunc(onReshape);
	glutDisplayFunc(onDisplay);

	glutMainLoop();
}
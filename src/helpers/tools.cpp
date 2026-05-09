#include "tools.h"

#include<iostream>
#include<string>
#include<fstream>
#include<vector>

#define STB_IMAGE_IMPLEMENTATION // макрос для включения функций stb_image
#include"stb_image.h"

#include<glad/glad.h>
#include<GLFW/glfw3.h>

#ifdef TARGET_OS_OSX
std::string PathOnMac()
{
	uint32_t size = 0;
	_NSGetExecutablePath(nullptr, &size);
	std::string exePath(size, '\0');
	_NSGetExecutablePath(exePath.data(), &size);
	bool chek = false;
	for (int i = exePath.size() - 1; i > 0; i--)
	{
		if (exePath[i] == '/')
		{
			if (chek) {
				exePath.resize(i + 1);
				break;
			}
			else
				chek = true;
		}

	}
	return exePath;
}
#endif

const char* tools::file_tools::loadTextFromFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::binary);

	if (!file.is_open()) {
		std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
		return nullptr;
	}

	// Получаем размер файла
	file.seekg(0, std::ios::end);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	// Выделяем память под содержимое файла + нуль-терминатор
	char* buffer = new char[size + 1];

	// Читаем файл
	file.read(buffer, size);
	buffer[size] = '\0'; // Добавляем нуль-терминатор

	file.close();
	return buffer;
}


bool tools::gl_tools::loadTexture(char const* filename, GLuint& texture, GLenum tex_type, GLuint filter, GLuint wrap, GLint aligment,
	bool auto_type, GLenum format)
{
	stbi_set_flip_vertically_on_load(true);	// настройка stb_image (переворачиваем Y, чтобы соответствовать OpenGL)

	int width, height, channels; // переменные параметров изображения
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 0); // загружаем все данные изображения

	// проверка коректной загрузки
	if (!data) {
		std::cout << "Не удалось загрузить текстуру!" << std::endl;
		return false;
	}

	//GLuint texture; // создаем идентификатор текстуры
	glGenTextures(1, &texture); // говорим создать Textures в количестве 1 и положить (id) идентификатор в VAO
	glBindTexture(tex_type, texture); // берет объект texture в контекст редактирования с типом привязки GL_TEXTURE_2D -<(тип привязки так же важен в типе переменной шейдера)

	if (auto_type)
		format = (channels == 4) ? GL_RGBA : GL_RGB; // проверяем количество каналов


	// настройки поведения текстуры типа GL_TEXTURE_2D
	glPixelStorei(GL_UNPACK_ALIGNMENT, aligment);
	glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, filter); // GL_TEXTURE_MIN_FILTER - поведение при уменьшении текстуры
	glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, filter); // GL_TEXTURE_MAG_FILTER - поведение при увеличении текстуры
	// GL_NEAREST:     GL_LINEAR:
	// ████████████    ░▒▓█████▓▒░
	glTexParameteri(tex_type, GL_TEXTURE_WRAP_S, wrap); // GL_TEXTURE_WRAP_S - по горизонтали (U координата) X
	glTexParameteri(tex_type, GL_TEXTURE_WRAP_T, wrap); // GL_TEXTURE_WRAP_T - по вертикали (V координата) Y
	glTexImage2D(tex_type, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); // загружаем данные в GPU
	// (1) - GL_TEXTURE_2D - тип текстуры
	// (2) - уровень mipmap(0 = основное изображение)
	// (3) - как OpenGL хранит данные в GPU (внутренний формат) (еще можно зрнить как сжатый RGB - GL_COMPRESSED_RGB)
	// (4) - ширина
	// (5) - высота
	// (6) - 0 - граница(устаревший параметр, всегда 0)
	// (7) - как интерпретировать твои данные (в каком формате будет вывод)
	// (8) - GL_UNSIGNED_BYTE - тип данных(0 - 255 для каждого канала)
	// (9) - data - указатель на пиксели
	stbi_image_free(data); // удаляем данные из оператики после загрузки на GPU

	return true;
}

bool tools::gl_tools::loadFlipTexture(unsigned char* data, int& width, int& height,
	GLuint& texture, GLenum tex_type, GLuint filter, GLuint wrap, GLint aligment,
	GLenum format)
{
	std::vector<unsigned char> flipped(width * height);
	for (int y = 0; y < height; ++y)
		memcpy(&flipped[y * width], &data[(height - 1 - y) * width], width);

	glGenTextures(1, &texture);
	glBindTexture(tex_type, texture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, aligment);
	glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(tex_type, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(tex_type, GL_TEXTURE_WRAP_T, wrap);
	glTexImage2D(tex_type, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	return true;
}

void tools::gl_tools::createVertexArrayOnElements(GLuint& VAO, GLuint& VBO, GLuint& EBO,
	GLenum draw_type, GLboolean normalized,
	GLsizeiptr size_of_vertices, const void* vertices,
	GLsizeiptr size_of_indices, const void* indices,
	GLsizeiptr size_of_attributes, int* attributes)
{
	glGenVertexArrays(1, &VAO); // говорим создать VertexArrays в количестве 1 и положить (id) идентификатор в VAO
	glBindVertexArray(VAO); // активируем VAO для записи конфигурации (VBO, EBO и тд...)

	createBufferObject(VBO, GL_ARRAY_BUFFER, size_of_vertices, vertices, draw_type);
	createBufferObject(EBO, GL_ELEMENT_ARRAY_BUFFER, size_of_indices, indices, draw_type);

	long long int count_attributes_of_one_element = 0;
	for (int i = 0; i < size_of_attributes; i++)
	{
		count_attributes_of_one_element += attributes[i];
	}

	int offset = 0;
	for (int i = 0; i < size_of_attributes; i++)
	{
		glVertexAttribPointer(i, attributes[i], GL_FLOAT, normalized, count_attributes_of_one_element * sizeof(GLfloat), (void*)(offset * sizeof(GLfloat)));
		glEnableVertexAttribArray(i);
		offset += attributes[i];
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0); // берем в контекст GL_ARRAY_BUFFER 0 - то несть ничего - то есть контекст чист
	glBindVertexArray(0);  // берем в главный контекст 0 - то несть ничего - то есть контекст чист
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // берем в контекст GL_ELEMENT_ARRAY_BUFFER 0 - то несть ничего - то есть контекст чист
}
void tools::gl_tools::createBufferObject(GLuint& glBO, GLenum BOtype, GLsizeiptr size_of_data, const void* data, GLenum draw_type)
{
	glGenBuffers(1, &glBO);
	glBindBuffer(BOtype, glBO); // берем в контекст GL_ARRAY_BUFFER с id - VBO
	glBufferData(BOtype, size_of_data, data, draw_type);
}
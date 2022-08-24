
#include <stdio.h>
#include <iostream>
#include <time.h> 
#include <windows.h> 
#include <conio.h>  
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include "resource.h"



//////////////////////////////////////////////////////////////

using namespace std;

// Enum цветов

enum ConsoleColor
{
	Black = 0,
	Blue = 1,
	Green = 2,
	Cyan = 3,
	Red = 4,
	Magenta = 5,
	Brown = 6,
	LightGray = 7,
	DarkGray = 8,
	LightBlue = 9,
	LightGreen = 10,
	LightCyan = 11,
	LightRed = 12,
	LightMagenta = 13,
	Yellow = 14,
	White = 15
};



//////////////////////////////////////////////////////////////

HANDLE hConsole;
HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

void SetColor(ConsoleColor text, ConsoleColor background)
{
	// Меняем цвет в консоли

	SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}

//////////////////////////////////////////////////////////////

void ShowConsoleCursor(bool showFlag)
{
	//Функция того, чтобы скрыть подчёркивание в консоли

	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(hStdOut, &cursorInfo);
	cursorInfo.bVisible = showFlag;
	SetConsoleCursorInfo(hStdOut, &cursorInfo);
}

//////////////////////////////////////////////////////////////


void Font() {

	CONSOLE_FONT_INFOEX fontInfo;

	// Функция того чтобы изменить шрифт и его размер

	fontInfo.cbSize = sizeof(fontInfo);

	GetCurrentConsoleFontEx(hConsole, TRUE, &fontInfo);

	wcscpy_s(fontInfo.FaceName, L"Lucida Console");

	fontInfo.dwFontSize.Y = 15;
	SetCurrentConsoleFontEx(hConsole, TRUE, &fontInfo);

}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

struct Snake  // структура змейка 
{
	COORD* snake_arr; //точки
	int snake_count; //количество яблок
};

struct Game
{
	Snake game_snake; //змейка
	COORD apple; //яблоко
	int snake_x, snake_y, speed; //задержка
	int direct; //направление

};

struct Player {

	string nickname; // ник
	ConsoleColor color_snake; // цвет змеи
	ConsoleColor color_wall; // цвет стен

};

struct File_str {
	string nickname; // ник
	int score; // очки
};

//////////////////////////////////////////////////////////////

//Enum направлений и объектов игры

enum uprawlenie { LEFT, UP, RIGHT, DOWN };
enum { END, PLUS, MOVE, PAUSE };

//////////////////////////////////////////////////////////////

void Draw_point(int X, int Y)
{
	// Ставим курсор туда куда нам нужно
	COORD coord = { X, Y };
	SetConsoleCursorPosition(hStdOut, coord);
}

//////////////////////////////////////////////////////////////

bool FileIsExist(string filePath)
{
	// Функция если файл включен

	bool isExist = false;
	ifstream fin(filePath.c_str());

	if (fin.is_open()) isExist = true;

	fin.close();
	return isExist;
}

//////////////////////////////////////////////////////////////

File_str Load_File(int iter) {

	ifstream file("save.txt", ios::out | ios::app); int count = 0; // Открываем файл с флагами не перезаписывать его
	string str; File_str array[5];

	while (getline(file, str))
	{
		// Проходимся по каждоый строчке файла и делим её пополам перебрасываю в массив

		string s = str;

		vector<string> vector_s;
		istringstream iss(str);
		copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(vector_s));

		// Честно говоря это единственная часть всего этого кода которую мне тяжеловато понять
		// Единственное что я тут понимаю это то что мы с помощью потоков делим вектор

		array[count].nickname = vector_s[0]; array[count].score = stoi(vector_s[1]);

		count += 1;


	}

	//Это мой костыль. Я не смог сделать так чтобы функция возвращала массив структур поэтому пошёл покостыльному 

	return array[iter];


}

//////////////////////////////////////////////////////////////

// Функция сортировки по убыванию для qsort. Cортируется на основе score

static int compare(const void* a, const void* b)
{
	return ((File_str*)b)->score - ((File_str*)a)->score;
}

//////////////////////////////////////////////////////////////

void Save_File(int score, Player player) {

	// Функция сохранения

	if (!FileIsExist("save.txt")) {

		// Если файла не существует впринципе то я его создаю заново

		ofstream file("save.txt", ios::out | ios::app);

		if (file.is_open()) {

			file << player.nickname << " " << score << endl;
			for (int i = 0; i < 4; i++) file << "Пустые-ячейки" << " " << "0" << endl;
		}

	}

	else {

		// Если он есть, то я открываю его без флагов перезаписывания

		ofstream file("save.txt", ios::out | ios::app);  File_str array[6];

		for (int i = 0; i < 5; i++) array[i] = Load_File(i);
		for (int i = 0; i < 5; i++) if (array[i].nickname == player.nickname) if (array[i].score >= score) return;

		// Если мои я уже есть в таблице и мои меньше того что там  то я ничего не делаю

		file.close(); ofstream file_new("save.txt");

		// После этого файл закрываю и открываю уже с флагом того, чтобы перезаписать


		if (file_new.is_open())

			array[5].nickname = player.nickname; array[5].score = score;

		// Я добавляю в конец этого массива структур свой ник и очки  и затем сортирую

		qsort(array, 6, sizeof(File_str), compare);

		// После этого снова завожу это в файл

		for (int i = 0; i < 5; i++) file_new << array[i].nickname << " " << array[i].score << endl;

		return;

	}
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void Draw_wall(ConsoleColor color_wall)
{
	// Отрисовка стены

	SetConsoleTextAttribute(hConsole, color_wall);
	for (int i = 1; i <= 61; i += 2) { Draw_point(i, 25); cout << "*" << endl; Draw_point(i, 1); cout << "*" << endl; }
	for (int i = 2; i <= 24; i++) { Draw_point(1, i); cout << "*" << endl; Draw_point(61, i); cout << "*" << endl; }
}

//////////////////////////////////////////////////////////////

void Apple_create(Game& object_game)
{

	int i, x, y; int count = object_game.game_snake.snake_count;
	do
	{
		x = rand() % 56 + 3; y = rand() % 19 + 3;  // Делаем рандомное появление 
		for (i = 0; i < count; i++) { if (x == object_game.game_snake.snake_arr[i].X && y == object_game.game_snake.snake_arr[i].Y) break; } // Cмотрим не наехало ли яблоко на змею
	} while (i < count);

	object_game.apple.X = x;
	object_game.apple.Y = y;

	SetConsoleCursorPosition(hConsole, object_game.apple);
	SetConsoleTextAttribute(hConsole, Red); printf("#");

}

//////////////////////////////////////////////////////////////

void StartGame(Game& object_game)
{
	// Инициализация и отрисовка перед началом игры

	system("cls");
	object_game.game_snake.snake_count = 3; object_game.game_snake.snake_arr = new COORD[3];

	for (int i = 0; i < 3; i++) { object_game.game_snake.snake_arr[i].X = 20 + i; object_game.game_snake.snake_arr[i].Y = 20; }


	object_game.snake_x = 1; object_game.snake_y = 0; object_game.speed = 100;
	Apple_create(object_game);
}

//////////////////////////////////////////////////////////////

int Move(Game& object_game, ConsoleColor color_snake, bool pause)
{
	// Основной цикл игры

	SetConsoleTextAttribute(hConsole, White);
	Draw_point(19, 0); cout << "Очки -> " << object_game.game_snake.snake_count - 3;
	if (pause) { SetConsoleTextAttribute(hConsole, Red); }
	else { SetConsoleTextAttribute(hConsole, White); } cout << "     " << " Пауза -> P";

	// н - локальная переменная длины змеи

	int& n = object_game.game_snake.snake_count;
	SetConsoleTextAttribute(hConsole, Green);

	COORD head = object_game.game_snake.snake_arr[n - 1];
	COORD tail = object_game.game_snake.snake_arr[0];
	COORD next;

	// Создаём координаты головы хвоста и следующего за головой объекта


	next.X = head.X + object_game.snake_x;
	next.Y = head.Y + object_game.snake_y;


	if (next.X < 2 || next.Y < 2 || next.X > 60 || next.Y > 24) return END; // Если змея ударилась в стену

	for (int i = 0; i < n; i++) if (next.X == object_game.game_snake.snake_arr[i].X && next.Y == object_game.game_snake.snake_arr[i].Y) return END; // Если змея ударилась в саму себя


	SetConsoleTextAttribute(hConsole, color_snake);

	//Если игра не на паузе идём дальше

	if (!pause) {

		// Если змея съела яблоко

		if (next.X == object_game.apple.X && next.Y == object_game.apple.Y)
		{
			// Переинициализация длины змеи

			COORD* temp = new COORD[++n];
			for (int i = 0; i < n; i++) temp[i] = object_game.game_snake.snake_arr[i];
			temp[n - 1] = next;
			delete[] object_game.game_snake.snake_arr; // Очистка памяти
			object_game.game_snake.snake_arr = temp;

			SetConsoleCursorPosition(hConsole, head); printf("O");

			Apple_create(object_game); return PLUS;
		}

		// Отрисовка змеи

		for (int i = 0; i < n - 1; i++) object_game.game_snake.snake_arr[i] = object_game.game_snake.snake_arr[i + 1]; object_game.game_snake.snake_arr[n - 1] = next;

		SetConsoleCursorPosition(hConsole, tail); printf(" ");

		SetConsoleCursorPosition(hConsole, head); printf("O");

		SetConsoleCursorPosition(hConsole, next); printf("O");



		return MOVE;
	}

	else { return MOVE; }


}

//////////////////////////////////////////////////////////////

void Game_cycle(Player player) {

	int key = 0, count = 0; Game object_game; bool pause = false;

	StartGame(object_game);
	Draw_wall(player.color_wall);

	while (key != 27)
	{
		while (!_kbhit()) // Пока клавиша не нажата
		{
			switch (Move(object_game, player.color_snake, pause))
			{
			case PLUS: // Вызывается если змея съела яблоко
				++count; object_game.speed -= 1; break; Draw_point(17, 0);



			case END: _getch(); system("cls"); Save_File(object_game.game_snake.snake_count - 3, player); return; // Конец игры. Сохранение файла

			}

			Sleep(object_game.speed); //Задержка
		}


		key = _getch();

		// Смена направления в зависимости от нажатой клавиши

		if (key == 72 && object_game.direct != DOWN)
		{
			object_game.direct = UP;
			object_game.snake_x = 0;
			object_game.snake_y = -1;
		}
		else if (key == 80 && object_game.direct != UP)
		{
			object_game.direct = DOWN;
			object_game.snake_x = 0;
			object_game.snake_y = 1;
		}
		else if (key == 75 && object_game.direct != RIGHT)
		{
			object_game.direct = LEFT;
			object_game.snake_x = -1;
			object_game.snake_y = 0;
		}
		else if (key == 77 && object_game.direct != LEFT)
		{
			object_game.direct = RIGHT;
			object_game.snake_x = 1;
			object_game.snake_y = 0;
		}

		else if (key == 'P' || key == 'p') {
			if (pause) {
				pause = false;
			}
			else {
				pause = true;
			}
		}


	}
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void Leaderboard(Player player) {

	system("cls"); File_str file[5];

	// Таблица лидеров. Получается за счёт вывода массива структур. Который мы заполняем через функцию LoadFile

	for (int i = 0; i < 5; i++) file[i] = Load_File(i);

	Draw_point(20, 5); cout << "Таблица лидеров" << endl;
	for (int i = 0; i < 5; i++) {
		Draw_point(15, 5 + (i + 1) * 3); cout << "   " << i + 1 << " -> " << file[i].nickname << " " << file[i].score << endl;
	}

	while (1) { switch (_getch()) { case 13:  system("cls");  return; } }

}

Player Options(Player player) {

	system("cls");

	// Настройки. Тоже в дополнительных комментариях не нуждается

	string color[16] = { "Чёрный", "Синий", "Зелёный", "Сине-зеленый", "Красный", "Маджента", "Коричневый", "Светло-серый", "Тёмно-серый", "Голубой", "Салатовый", "Светлый сине-зелёный","Светлый красный", "Светлый Маджента", "Жёлтый", "Белый" };

	COORD select;

	select.X = 3; select.Y = 5;

	Draw_point(select.X, select.Y); cout << ">";

	Draw_point(4, 2); cout << "Настройки" << endl;
	Draw_point(4, 5); cout << "Имя пользователя" << endl; Draw_point(25, 5); cout << player.nickname << endl;
	Draw_point(4, 8); cout << "Цвет змейки" << endl; Draw_point(20, 8); cout << color[player.color_snake] << endl;
	Draw_point(4, 11); cout << "Цвет стен" << endl; Draw_point(20, 11); cout << color[player.color_wall] << endl;
	Draw_point(4, 14); cout << "Выход" << endl;

	int count = player.color_snake;
	int count2 = player.color_wall;

	string nickname = player.nickname;

	while (1)
	{


		int key = _getch();

		if (key == 72)
		{
			for (int i = 1; i <= 50; i++) {

				Draw_point(3, i); cout << " ";

			}

			if (select.Y > 5) {
				select.Y -= 3;
			}

			else if (select.Y <= 5) {

				select.Y = 14;
			}



		}
		else if (key == 80)
		{
			for (int i = 1; i <= 50; i++) {

				Draw_point(3, i); cout << " ";

			}

			if (select.Y < 14) {
				select.Y += 3;
			}

			else if (select.Y >= 14) {

				select.Y = 5;
			}

		}
		else if (key == 75)
		{


			if (select.Y == 8) {
				for (int i = 20; i < 40; i++) { Draw_point(i, 8); cout << " "; }
				Draw_point(20, 8);
				count -= 1;
				if (count < 0) {
					count = 15;
				}
				cout << color[count] << endl;
			}

			else if (select.Y == 11) {

				for (int i = 20; i < 40; i++) { Draw_point(i, 11); cout << " "; }

				Draw_point(20, 11);
				count2 -= 1;
				if (count2 < 0) {
					count2 = 15;
				}
				cout << color[count2] << endl;
			}
		}
		else if (key == 77)
		{


			if (select.Y == 8) {

				for (int i = 20; i < 40; i++) { Draw_point(i, 8); cout << " "; }

				Draw_point(20, 8);
				count -= 1;
				if (count < 0) {
					count = 15;
				}
				cout << color[count] << endl;
			}

			else if (select.Y == 11) {
				for (int i = 20; i < 40; i++) { Draw_point(i, 11); cout << " "; }
				Draw_point(20, 11);
				count2 += 1;
				if (count2 > 15) {
					count2 = 0;
				}
				cout << color[count2] << endl;
			}
		}

		else if (key == 13)
		{
			if (select.Y == 5) {
				for (int i = 25; i < 40; i++) { Draw_point(i, 5); cout << " "; }
				Draw_point(25, 5); ShowConsoleCursor(true); cin >> nickname; ShowConsoleCursor(false);
			}

			else if (select.Y == 14) {

				player.nickname = nickname;
				player.color_snake = (ConsoleColor)count;
				player.color_wall = (ConsoleColor)count2;
				system("cls");
				return player;
			}


		}


		Draw_point(select.X, select.Y); cout << ">";


	}
}



Player Menu(Player player) {

	// Меню основное. Отрисовка и нажатие клавиш думаю не нуждается в комментариях

	COORD select; SetConsoleTextAttribute(hConsole, White);

	select.X = 18; select.Y = 10;
	Draw_point(select.X, select.Y); cout << ">";

	Draw_point(20, 7); cout << "ЗМЕЙКА" << endl;
	Draw_point(20, 10); cout << "Игра" << endl;
	Draw_point(20, 13); cout << "Таблица лидеров" << endl;
	Draw_point(20, 16); cout << "Настройки" << endl;

	while (1)
	{
		switch (_getch()) {
		case 72:
			for (int i = 1; i <= 50; i++) {

				Draw_point(18, i); cout << " ";

			}

			if (select.Y > 10) {
				select.Y -= 3;
			}

			else if (select.Y <= 10) {

				select.Y = 16;
			}


			break;
		case 80:

			for (int i = 1; i <= 50; i++) {

				Draw_point(18, i); cout << " ";

			}

			if (select.Y < 16) {
				select.Y += 3;
			}

			else if (select.Y >= 16) {

				select.Y = 10;
			}
			break;
		case 13:
			if (select.Y == 10) { Game_cycle(player); return player; }

			else if (select.Y == 13) { Leaderboard(player); return player; }

			else if (select.Y == 16) { return Options(player); }

		}
		Draw_point(select.X, select.Y); cout << ">";
	}

}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void main()
{
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE); system("mode con cols=64 lines=27"); ShowConsoleCursor(false); // Получаем дестрибутив консоли. Задаём ей размер и убераем курсор

	Player player; player.nickname = "Игрок"; player.color_snake = Green; player.color_wall = White; // Инициализация основных переменных

	SetConsoleCP(1251);  SetConsoleOutputCP(1251); // Русский язык ввод\вывод

	Save_File(0, player); // Создание файла если его не было до этого

	while (1) { player = Menu(player); } // Цикл постоянной менюшки. А также постоянной переинициализации структуры игрока. После каждой игры.

}
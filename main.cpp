#include <SDL2/SDL.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Enums & Structs
enum class GAME_STATE{PLAY,
		      QUIT};

struct tM {
	float fR; // FrameRate
	float fT; // FrameTime 1000/fR
	float tf; // Test frames for displaying framerate (so that it does not stutter around randomly
	float ct; // Current time (ticks)
	float ot; // Last time (ticks)
	float dt; // Delta time (actual time)
};

struct v2 {
	int x;
	int y;
};

struct ConF
	{
		int resX;
		int resY;
		int mapSize;
		int cubeSize;
		int type;
	};

// Prototypes
void init(int fps);
class Init
{
public:
	static void initSystems(SDL_Window* window, SDL_Renderer* renderer);
	static void initDefaults(tM* t, int fps);
};
void gameLoop();
void processInput();
void update();
void draw();
void limitFPS(tM* t);
void fatalError(std::string errorString);
void initMaze();
void saveToFile();
void aStar();


// Config

void loadFileIntoBuffer(std::string filePath, std::vector<char>& buffer);
void loadConfig(std::string filePath);
int reduceRepitionNotSureWhatToCallThis(std::string par, std::string sBuf);

class Cell
{
public:
	//Cell(int x, int y);
	void setPos(int x, int y, int size);
	void draw(SDL_Renderer* _renderer);
	
	v2 pos;
	bool wall[4];
	bool visited;
	SDL_Rect dWall[4];
	SDL_Rect dVisit;
};

// Stuff
SDL_Window* _window;
SDL_Renderer* _renderer;
SDL_Event e;
GAME_STATE _gameState;
tM t;
Cell* cell;
int c_amount;	//59;
int cellSize;	//15;
int currentCell;
int unvisited = c_amount*c_amount-1;
std::vector<int> cStack;
std::vector<int> neighboursUnvisited;
ConF config;
time_t seed;
bool saveFlag;
int typeflag;

int main (int argc, char* argv[])
{
	saveFlag = argc > 2;
	loadConfig ("../../config.cfg");
	if (config.mapSize)
	{
		c_amount = config.mapSize;
		cellSize = config.cubeSize;
		unvisited = c_amount*c_amount-1;
	}
	cell = new Cell[c_amount*c_amount];
	for (int i = 0; i < c_amount; i++)
	{
		for (int j = 0; j < c_amount; j++)
			cell[j*c_amount+i].setPos(i, j, cellSize);
	}
	std::cout << argc << '\n';
	if (argc == 1)
		init(60.0f);
	else
		init(std::stoi(argv[1]));
	if (!saveFlag)
		gameLoop();
	return 0;
};

void init(int fps)
{
	if(SDL_Init(SDL_INIT_EVERYTHING))
		fatalError("SDL could not be initialized!");
	_window = SDL_CreateWindow("Maze", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				  		config.resX, config.resY, SDL_WINDOW_BORDERLESS);
	if (!_window)
		fatalError("SDL Window could not be created!");
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	if (!_renderer)
		fatalError("SDL Renderer could not be created!");
	//Init::initSystems(_window, _renderer);
	Init::initDefaults(&t, fps);
	
	seed = time(NULL);
	srand(seed); //Seed Rand
	
	initMaze();
	
	if (!saveFlag)
	{
		SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
		for (int i = 0; i < c_amount*c_amount; i++)
			cell[i].draw(_renderer);
	}
}

void gameLoop()
{
	currentCell = 0;
	cell[0].visited = true;
	
	while (_gameState != GAME_STATE::QUIT)
	{	
		processInput();
		update();
		draw();
		limitFPS(&t);
	}
}

void limitFPS(tM* t)
{
	t->ct = SDL_GetTicks();
	if (t->ct - t->ot < t->fT)
	{
		SDL_Delay(t->fT - t->ct + t->ot);
		//std::cout << '\n' << (int)(t->fT - t->ct + t->ot);
	}
	t->ot = t->ct;
	//t->dt = 
}


void processInput()
{
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
					_gameState = GAME_STATE::QUIT;
				break;
			case SDL_KEYDOWN:
					switch (e.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							_gameState = GAME_STATE::QUIT;
						break;
						case SDLK_o:
							saveToFile();
					}
				break;
			case SDL_KEYUP:
				break;
			default:
				break;
		}
	}
}

void update()
{
	//std::cout << cell[currentCell].pos.x << '\n';
	
}

void draw()
{
	//SDL_SetRenderDrawColor(_renderer,0,0,0,250);
	//SDL_RenderClear(_renderer);
	//for (int i = 0; i < c_amount*c_amount; i++)
		//cell[i].draw(_renderer);
	cell[currentCell].draw(_renderer);
	SDL_RenderPresent(_renderer);
}

void initMaze()
{
	while (unvisited)
	{
		neighboursUnvisited.resize(0);
		v2 cPos;
		cPos.x = cell[currentCell].pos.x;
		cPos.y = cell[currentCell].pos.y;
		if (cPos.y && !cell[(cPos.y-1)*c_amount+cPos.x].visited)		// Above
			neighboursUnvisited.push_back((cPos.y-1)*c_amount+cPos.x);
		if (cPos.y < c_amount-1 && !cell[(cPos.y+1)*c_amount+cPos.x].visited)	// Below
			neighboursUnvisited.push_back((cPos.y+1)*c_amount+cPos.x);
		if (cPos.x && !cell[cPos.y*c_amount+(cPos.x-1)].visited)		// Left
			neighboursUnvisited.push_back(cPos.y*c_amount+(cPos.x-1));
		if (cPos.x < c_amount-1 && !cell[cPos.y*c_amount+(cPos.x+1)].visited)	// Right
			neighboursUnvisited.push_back(cPos.y*c_amount+(cPos.x+1));
		if (neighboursUnvisited.size())
		{
			int r = rand() % neighboursUnvisited.size();
			//std::cout << (int)r << '\n';
			int nC = neighboursUnvisited[r];
			//std::cout << "X: " << cell[nC].pos.x << " Y: " << cell[nC].pos.y << '\n'; 
			cStack.push_back(currentCell);
			if (cPos.y > cell[nC].pos.y)						// Above
			{
				cell[currentCell].wall[0] = false;
				cell[nC].wall[2] = false;
			}
			else if (cPos.y < cell[nC].pos.y)					// Below
			{
				cell[currentCell].wall[2] = false;
				cell[nC].wall[0] = false;
			}
			else
			{
				if (cPos.x > cell[nC].pos.x)					// Left
				{
					cell[currentCell].wall[3] = false;
					cell[nC].wall[1] = false;
				}
				else if (cPos.x < cell[nC].pos.x)				// Right
				{
					cell[currentCell].wall[1] = false;
					cell[nC].wall[3] = false;
				}
			}
			currentCell = nC;
			cell[currentCell].visited = true;
			unvisited--;
		}
		else if (cStack.size())
		{
			currentCell = cStack.back();
			cStack.pop_back();
		}
	}
	
	if (saveFlag)
	{
		saveToFile();
	}
};







// Cell

void Cell::setPos(int x, int y, int size)
{
	int offset = size;
	this->pos.x = x;
	this->pos.y = y;
	this->visited = false;
	
	for (int i = 0; i < 4; i++)
	{
		this->wall[i] = true;
	}
	
	this->dWall[0].x = this->pos.x*size-size/2+offset;
	this->dWall[0].y = this->pos.y*size-size/2+offset;
	this->dWall[0].h = 1;
	this->dWall[0].w = size;
	
	this->dWall[1].x = this->pos.x*size+size/2+offset;
	this->dWall[1].y = this->pos.y*size-size/2+offset;
	this->dWall[1].h = size;
	this->dWall[1].w = 1;
	
	this->dWall[2].x = this->pos.x*size-size/2+offset;
	this->dWall[2].y = this->pos.y*size+size/2+offset;
	this->dWall[2].h = 1;
	this->dWall[2].w = size;
	
	this->dWall[3].x = this->pos.x*size-size/2+offset;
	this->dWall[3].y = this->pos.y*size-size/2+offset;
	this->dWall[3].h = size;
	this->dWall[3].w = 1;
	
	this->dVisit.x = this->pos.x*size-size/2+offset;
	this->dVisit.y = this->pos.y*size-size/2+offset;
	this->dVisit.h = size;
	this->dVisit.w = size;
}

void Cell::draw(SDL_Renderer* _renderer)
{
	//std::cout << '\n' << this->pos.x << ' ' << this->pos.y;
	/*
	if (this->visited)
	{
		SDL_Color c;
		SDL_GetRenderDrawColor(_renderer, &c.r, &c.g, &c.b, &c.a);
		SDL_SetRenderDrawColor(_renderer, 200, 40, 0, 130);
		SDL_RenderFillRect(_renderer, &dVisit);
		SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
	}
	if (this->pos.y*c_amount+this->pos.x == currentCell)
	{
		SDL_Color c;
		SDL_GetRenderDrawColor(_renderer, &c.r, &c.g, &c.b, &c.a);
		SDL_SetRenderDrawColor(_renderer, 0, 0, 255, 70);
		SDL_RenderFillRect(_renderer, &dVisit);
		SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
	}
	*/
	SDL_Color c;
	SDL_GetRenderDrawColor(_renderer, &c.r, &c.g, &c.b, &c.a);
	SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 70);
	SDL_RenderFillRect(_renderer, &dVisit);
	SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
	this->wall[0] ? SDL_RenderDrawRect(_renderer, &dWall[0]) : 0;
	this->wall[1] ? SDL_RenderDrawRect(_renderer, &dWall[1]) : 0;
	this->wall[2] ? SDL_RenderDrawRect(_renderer, &dWall[2]) : 0;
	this->wall[3] ? SDL_RenderDrawRect(_renderer, &dWall[3]) : 0;
}

void saveToFile()
{
	FILE *file;
	std::string path = "./maze";
	path += std::to_string(seed);
	std::string contents;
	if (config.type == 0)
	{	//	PPM - P6
		path += ".ppm";
		file = fopen(path.c_str(), "wb");

		contents += "P6\n";
		contents += std::to_string(c_amount*2+1);
		contents += '\n';
		contents += std::to_string(c_amount*2+1);
		contents += "\n255\n";

		std::string white;
		white += (char)255;
		white += (char)255;
		white += (char)255;

		std::string red;
		red += (char)255;
		red += (char)1;
		red += (char)1;

		for (int i = 0; i < (c_amount*2+1); i++)
			contents += white;


		for (int i = 0; i < c_amount; i++)
		{
			contents += white;
			for (int j = 0; j < c_amount; j++)
			{
				contents += red;
				if (cell[i*c_amount+j].wall[1])
				{
					contents += white;
				}
				else
				{
					contents += red;
				}
			}
			contents += white;
			for (int j = 0; j < c_amount; j++)
			{
				if (cell[i*c_amount+j].wall[2])
				{
					contents += white;
				}
				else
				{
					contents += red;
				}
				contents += white;
			}
		}
	}
	else if (config.type == 1)
	{
		path += ".pbm";

		file = fopen(path.c_str(), "wb");

		contents += "P1\n";
		contents += std::to_string(c_amount*2+1);
		contents += '\n';
		contents += std::to_string(c_amount*2+1);
		contents += '\n';

		char pbmwhite = '0';
		char pbmred = '1';

		for (int i = 0; i < (c_amount*2+1); i++)
			contents += pbmwhite;


		for (int i = 0; i < c_amount; i++)
		{
			contents += pbmwhite;
			for (int j = 0; j < c_amount; j++)
			{
				contents += pbmred;
				if (cell[i*c_amount+j].wall[1])
				{
					contents += pbmwhite;
				}
				else
				{
					contents += pbmred;
				}
			}
			contents += pbmwhite;
			for (int j = 0; j < c_amount; j++)
			{
				if (cell[i*c_amount+j].wall[2])
				{
					contents += pbmwhite;
				}
				else
				{
					contents += pbmred;
				}
				contents += pbmwhite;
			}
			contents += '\n';
		}
	}
	else if (config.type == 2)
	{
		path += ".pbm";

		file = fopen(path.c_str(), "wb");

		

		char pbmwhite = '0';
		char pbmred = '1';

		for (int i = 0; i < (c_amount*2+1); i++)
			contents += pbmwhite;


		for (int i = 0; i < c_amount; i++)
		{
			contents += pbmwhite;
			for (int j = 0; j < c_amount; j++)
			{
				contents += pbmred;
				if (cell[i*c_amount+j].wall[1])
				{
					contents += pbmwhite;
				}
				else
				{
					contents += pbmred;
				}
			}
			contents += pbmwhite;
			for (int j = 0; j < c_amount; j++)
			{
				if (cell[i*c_amount+j].wall[2])
				{
					contents += pbmwhite;
				}
				else
				{
					contents += pbmred;
				}
				contents += pbmwhite;
			}
		}
		
		
		std::string con = contents;
		contents = "";
		char temp = 0;
		int track = 128;
		int tLength = 0;
		
		for (int i = 0; i < con.length(); i++)
		{
			if (con[i] == '1')
				temp += track;
			if (track == 1)
			{
				track = 128;
				contents += temp;
				temp = 0;
			}
			else
				track = track >> 1;
			tLength++;
			if (tLength == c_amount*2+1)
			{
				track = 128;
				contents += temp;
				temp = 0;
				tLength = 0;
			}
			
		}
		
		con = contents;
		
		contents = "";
		contents += "P4\n";
		contents += std::to_string(c_amount*2+1);
		contents += '\n';
		contents += std::to_string(c_amount*2+1);
		contents += '\n';
		fprintf(file, "%s", contents.c_str());
		printf("%d\n", con.length());
		fwrite(con.c_str(), 1, con.length(), file);
		fclose(file);
	}
	if (config.type != 2)
	{
		fprintf(file, "%s", contents.c_str());
		fclose(file);
		printf("Finished\n");
	}
};


// Init

void Init::initSystems(SDL_Window* window, SDL_Renderer* renderer)
{
	if(SDL_Init(SDL_INIT_EVERYTHING))
		fatalError("SDL could not be initialized!");
	window = SDL_CreateWindow("Maze", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				  		1280, 720, SDL_WINDOW_BORDERLESS);
	if (!window)
		fatalError("SDL Window could not be created!");
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		fatalError("SDL Renderer could not be created!");
}

void Init::initDefaults(tM* t, int fps)
{
	_gameState = GAME_STATE::PLAY;
	t->fR = fps;
	t->fT = (1000.0f / t->fR) * 2;
	t->tf = 0; // Not implementing atm;
	t->ct = 0;
	t->ot = 0;
	t->dt = 0;
}

void fatalError(std::string errorString)
{
	std::cout << "A fatal error has occured, details below: \n" << errorString << "\nPress any key to continue....";
	int tmp;
	std::cin >> tmp;
	SDL_Quit();
	exit(-1);
}








































void loadFileIntoBuffer(std::string filePath, std::vector<char>& buffer)
{
	
	std::ifstream file(filePath,std::ios::binary);
	if (file.fail())
		fatalError("Could not open file! " + filePath);


	file.seekg(0, std::ios::end);
	int fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	fileSize -= file.tellg();


	buffer.resize(fileSize);
	file.read(&(buffer[0]), fileSize);
	file.close();
}



void loadConfig(std::string filePath)
{
	// Load file
	std::vector<char> buf;
	loadFileIntoBuffer(filePath, buf);
	std::string sBuf;
	for (int i = 0; i < buf.size(); i++)
		sBuf += buf[i];


	// Load res x=config[0] y=config[1]
	int found = sBuf.find("res");
	if (!(found == -1))
	{
		int x = sBuf.find("x",found);
		if (x == -1)
			fatalError("Incorrect formatting in config file!");
		int y = sBuf.find(";",found);
		if (y == -1)
			fatalError("Incorrect formatting in config file!");
		if (x>y)
			fatalError("Incorrect formatting in config file!");
		std::string sub = sBuf.substr(found + 4, x);
		config.resX = std::stoi(sub);
		sub = sBuf.substr(x+1, y);
		config.resY = std::stoi(sub);
	}

	std::string list[] = {"mapSize" ,
							"cubeSize",
							"type"};
	config.mapSize = reduceRepitionNotSureWhatToCallThis(list[0], sBuf);
	config.cubeSize = reduceRepitionNotSureWhatToCallThis(list[1], sBuf);
	config.type = reduceRepitionNotSureWhatToCallThis(list[2], sBuf);
	// Return
}

int reduceRepitionNotSureWhatToCallThis(std::string par, std::string sBuf)
{
	double found = sBuf.find(par);
	if (!(found == -1))
	{
		int x = sBuf.find("=",found);
		if (x == -1)
			fatalError("Incorrect formatting in config file!");
		int y = sBuf.find(";",found);
		if (y == -1)
			fatalError("Incorrect formatting in config file!");
		if (x>y)
			fatalError("Incorrect formatting in config file!");
		std::string sub = sBuf.substr(x+1, y-x);
		found = std::stod(sub);
	}
		return found;
}





#include"board.hpp"
#include"utilities.hpp"
#include<algorithm>
#include<iostream>
#include<fstream>
#include<vector>

Board::Board(BoardConfig *config, AppearanceConfig& appearance)
{
    this->config = config;
    
    if(config->inputFilePath!="")
        loadFromFile();
    else if(config->draw)
    {
        initGameBoardArrays();
        clear();
    }
    else
    {
        initGameBoardArrays();
        fillWithRandomValues();
    }

    background.setFillColor(appearance.deadCellColor);
    block.setFillColor(appearance.liveCellColor);
}

Board::~Board()
{
    if(config->outputFilePath!="")
        saveToFile();
}

void Board::clear()
{
    for(int y = 0;y<config->height;y++)
        for(int x = 0;x<config->width;x++)
            setBlockValue(x, y, false);
}

void Board::fillWithRandomValues()
{
    srand(time(NULL));
    for(int y = 0;y<config->height;y++)
        for(int x = 0;x<config->width;x++)
            setBlockValue(x,y, std::rand()%2==1);
}

void Board::setBlockValue(int x, int y, bool value)
{
    if(x<0||y<0||x>=config->width||y>=config->height)
        return;
    gameBoard1[x+y*config->width]=value;
    gameBoard2[x+y*config->width]=value;
}

void Board::setBlockSize(unsigned int size)
{
    blockSize=std::max(size,config->minBlockSize);
    block = sf::RectangleShape(sf::Vector2f(blockSize,blockSize));
    background.setSize(sf::Vector2f(blockSize*config->width,blockSize*config->height));
}

void Board::step()
{
    for(int y = 0;y<config->height;y++)
        for(int x = 0;x<config->width;x++)
            processCell(x, y);

    equalizeArrays();
}

void Board::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    sf::RectangleShape block = this->block;
    target.draw(background);
    for(int y = 0;y<config->height;y++){
        for(int x = 0;x<config->width;x++)
        {
            if(gameBoard1[x+y*config->width])
            {
                block.setPosition(x*blockSize,y*blockSize);
                target.draw(block);
            }
        }
    }
}

void Board::initGameBoardArrays()
{
    int arraysSize = config->width*config->height;
    gameBoard1 = std::make_unique<bool[]>(arraysSize);
    gameBoard2 = std::make_unique<bool[]>(arraysSize);
}

void Board::loadFromFile()
{
    if(isPNGImage(config->inputFilePath))
        loadFromPNGImage();
    else{
        loadFromTextFile();
    }
}

void Board::loadFromPNGImage()
{
    png::image<png::rgb_pixel> image;
    try
    {
        image.read(config->inputFilePath);
    }
    catch(png::std_error error)
    {
        throw std::string(error.what());
    }
    catch(...)
    {
        throw std::string("Failed to open file '"+config->inputFilePath+"'");
    }
    config->width=image.get_width();
    config->height=image.get_height();
    initGameBoardArrays();
    for(int x=0; x<config->width; x++)
        for(int y=0; y<config->height; y++)
            setBlockValue(x, y, image[y][x]);
}

void Board::loadFromTextFile()
{
    std::fstream file(config->inputFilePath, std::ios::in);
    if(!file)
        throw std::string("Failed to open file '"+config->inputFilePath+"'");
    
    file>>*this;

    file.close();
}

void Board::saveToFile()
{
    if(isPNGImage(config->outputFilePath))
        saveToPNGImage();
    else
        saveToTextFile();
}

void Board::saveToPNGImage()
{
    png::image<png::rgb_pixel> image(config->width, config->height);
    for(int x=0; x<config->width; x++)
        for(int y=0; y<config->height; y++)
            image[y][x] = gameBoard1[x+y*config->width]
                ? png::rgb_pixel(255, 255, 255) : png::rgb_pixel(0, 0, 0);
    try
    {
        image.write(config->outputFilePath);
    }
    catch(png::std_error error)
    {
        std::cerr << error.what() << std::endl;
    }
}

void Board::saveToTextFile()
{
    std::fstream file(config->outputFilePath, std::ios::out);
    if(!file)
        std::cerr << "Failed to save to file '" << config->outputFilePath << "'" << std::endl;
    
    file<<*this;

    file.close();
}

int Board::countLivingNeighbours(int x, int y)
{
    int livingNeighbours=0;

    for(int j=y-1;j<=y+1;j++)
        for(int i=x-1;i<=x+1;i++)
            if(i>=0&&i<config->width&&j>=0&&j<config->height&&!(i==x&&j==y))
                if(gameBoard1[i+j*config->width])
                    livingNeighbours++;

    return livingNeighbours;
}

void Board::processCell(int x, int y)
{
    if(gameBoard1[x+y*config->width])
    {
        if(!config->survive.count(countLivingNeighbours(x,y)))
            gameBoard2[x+y*config->width]=false;
    }
    else
    {
        if(config->birth.count(countLivingNeighbours(x,y)))
            gameBoard2[x+y*config->width]=true;
    }
}

void Board::equalizeArrays()
{
    int arraysSize = config->height*config->width; 
    for(int i = 0; i<arraysSize; i++)
        gameBoard1[i]=gameBoard2[i];
}

void Board::setBlockValue(int x, int y, char value)
{
    switch(value)
    {
        case 'X':
            setBlockValue(x, y, true);
            break;
        case ' ':
            setBlockValue(x, y, false);
            break;
        default:
            throw "The board file should only contain 'X's and spaces";
            break;
    }
}

void Board::setBlockValue(int x, int y, png::rgb_pixel value)
{
    if(value==png::rgb_pixel(255, 255, 255))
        setBlockValue(x, y, true);
    else if (value==png::rgb_pixel(0, 0, 0))
        setBlockValue(x, y, false);
    else
        throw std::string("The board image should contain only black and white pixels");
}

std::fstream& operator<<(std::fstream& os, const Board& board)
{
    for(int y = 0;y<board.config->height;y++)
    {
        for(int x = 0; x<board.config->width; x++)
        {
            if(board.gameBoard1[x+y*board.config->width])
                os<<'X';
            else
                os<<' ';
        }
        if(y!=(board.config->height-1))
            os<<"\n";
    }
    return os;
}

void operator>>(std::fstream& fs, Board& board)
{
    std::vector<std::string> lines = loadNonEmptyLines(fs);

    board.config->width = lines[0].length();
    board.config->height = lines.size();

    board.initGameBoardArrays();

    for(int y=0; y<board.config->height; y++)
    {
        std::string line = lines[y];
        if(line.length()!=board.config->width)
            throw std::string("All lines in the board file must be the same lenght");
        for(int x=0; x<board.config->width; x++)
            board.setBlockValue(x, y, line[x]);
    }
}

std::vector<std::string> loadNonEmptyLines(std::fstream& fs)
{
    std::vector<std::string> lines;
    std::string line;
    while(getline(fs, line))
        lines.push_back(line);

    if(lines.empty())
        throw std::string("The board file is empty");
    else if(line.empty())
        throw std::string("The board file contains an empty line");

    return lines;
}
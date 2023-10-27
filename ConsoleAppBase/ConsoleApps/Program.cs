// See https://aka.ms/new-console-template for more information
Console.WriteLine("Hello, World!");




int xMin = 0, yMin = 0;
int xMax = 80, yMax = 30;
int x = 10, y = 10;
int xOld = x, yOld = y;

Console.SetWindowSize(1, 1);
Console.SetBufferSize(xMax + 1, yMax + 1);
Console.SetWindowSize(xMax + 1, yMax + 1);
Console.CursorSize = 100;

bool end = false;
while(!end)
{
    while(Console.KeyAvailable == false)
        Thread.Sleep(250);

    var cki = Console.ReadKey(false);
    switch(cki.Key)
    {
        case ConsoleKey.UpArrow: y--; break;
        case ConsoleKey.DownArrow: y++; break;
        case ConsoleKey.LeftArrow: x--; break;
        case ConsoleKey.RightArrow: x++; break;

        case ConsoleKey.Escape: end = true; break;
    }

    MinMax(ref x, ref y);
    PrintXY(xOld, yOld, ' ');
    PrintXY(x, y, '*');
    xOld = x;
    yOld = y;
    Console.ForegroundColor = ConsoleColor.Green;
}


void PrintXY(int x, int y, char chr)
{
    Console.SetCursorPosition(x, y);
    Console.Write(chr);
    Console.SetCursorPosition(x, y);
}

void MinMax(ref int x, ref int y)
{
    if(x < xMin) x = xMin;
    else if(x > xMax) x = xMax;

    if(y < yMin) y = yMin;
    else if(y > yMax) y = yMax;
}

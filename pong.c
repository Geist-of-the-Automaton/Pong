//main c file
#include "pong.h"

void sync()
{
	while(REG_DISPLAY_VCOUNT >= 160);
	while(REG_DISPLAY_VCOUNT < 160);
}

uint16 makeColor(uint8 r, uint8 g, uint8 b)
{
	return (r & 0x1f) | ((g & 0x1f) << 5) | ((b & 0x1f) << 10);
}

uint32 clamp(int value, int min, int max)
{
	return (value < min ? min : (value > max ? max : value));
}

int hasHit(int bl, int bt, struct Rect p)
{
	// see if it hit player 1
	return bl <= p.w && bt <= p.y + p.h && bt + 8 >= p.y;
}

int hasHit2(int bl, int bt, struct Rect p)
{
	// see if it hit player 2
	return bl >= p.x && bt <= p.y + p.h && bt + 8 >= p.y;
}

void drawRect(struct Rect rect, uint16 color)
{
	for(int y = 0; y < rect.h; y++)
		for(int x = 0; x < rect.w; x++)
			SCREENBUFFER[(rect.y + y) * SCREEN_WIDTH + rect.x + x] = color;
}

void drawSevenSeg(uint8 n, int player, uint16 color)
{
	// get w, x, y, and z and draw the seven seg. rects with letter used for each seg
	a.x = d.x = e.x = f.x = g.x = (SCREEN_WIDTH / 2) - 16 + (player * 21);
	b.x = c.x = a.x + 8;
	int w = n >> 3;
	int x = (n - (w << 3)) >> 2;
	int y = (n - ((w << 3) + (x << 2))) >> 1;
	int z = (n - ((w << 3) + (x << 2) + (y << 1)));
	if (y | w | (x & z) | (!x & !z))
		drawRect(a, color);
	if ((!y & !z) | (y & z) | !x)
		drawRect(b, color);
	if (!y | z | x)
		drawRect(c, color);
	if (w | (y & !z) | (!x & !z) | (!x & y) | (x & !y & z))
		drawRect(d, color);
	if (((y | !x) & !z))
		drawRect(e, color);
	if (w | (x & (!y | !z)) | (!y & !z))
		drawRect(f, color);
	if (w | (!y & x) | (y & (!x | !z)))
		drawRect(g, color);
}

void initSevenSeg()
{
	// initialize the 7 seg rect positions and dimensions
	a.h = g.h = d.h = b.w = c.w = e.w = f.w = a.y = f.y = b.y = 4;
	a.w = g.w = d.w = b.h = c.h = e.h = f.h = e.y = g.y = c.y = 12;
	d.y = 20;
}

void drawButton(struct Rect r, int w, int h, int y, int isActive, int player)
{
	uint16 color = isActive ? makeColor (0x16, 0x16, 0x16) : makeColor(0x1f, 0x1f, 0x1f);
	r.x = w + (player * 4 * w);
	r.w = 3 * w;
	r.h = y;
	r.y = h + 2 * y;
	drawRect(r, color);
	r.y += isActive ? 1 : 2;
	r.x += isActive ? 1 : 2;
	r.w -= isActive ? 2 : 4;
	r.h -= isActive ? 2 : 4;
	color = isActive ? makeColor (0x16, 0, 0) : makeColor(0x1f, 0x0f, 0x0f);
	drawRect(r, color);
	color = isActive ? makeColor(0x16, 0x16, 0x16) : makeColor(0x1f, 0x1f, 0x1f);

	if (player)
	{
		int sec = r.h / 4;
		int xmid = r.x + r.w / 2;
		int ymid = r.y + 2 * sec;
		r.x = xmid - 3;
		r.h = 2;
		r.w = 6;
		for (int i = 1; i >= 0; i--)
		{
			r.y = ymid + (i - 1) * sec;
			drawRect(r, color);
		}
		r.w = 2;
		r.h = 2 * sec + 2;
		drawRect(r, color);
		r.h = sec + 1;
		r.x += 5;
		drawRect(r, color);
		r.x -= 15;
		r.h *= 2;
		drawRect(r, color);
		r.h = 2;
		r.w = 6;
		drawRect(r, color);
		r.y += 2 * sec;
		drawRect(r, color);
		r.x += 21;
		drawRect(r, color);
		r.y = ymid - sec;
		r.w = 2;
		r.h = 2 * sec + 2;
		drawRect(r, color);
		r.x += 4;
		drawRect(r, color);
	}
	else
	{
		int sec = r.h / 4;
		int xmid = r.x + r.w / 2;
		int ymid = r.y + 2 * sec;
		r.x = xmid - 8;
		r.y = ymid + 1;
		r.h = r.h / 4 + 1;
		r.w = 2;
		drawRect(r, color);
		r.y -= sec + 1;
		r.x += 4;
		drawRect(r, color);
		r.x = xmid - 8;
		r.y = ymid;
		r.h = 2;
		r.w = 6;
		for (int i = 0; i < 3; i++)
		{
			r.y = ymid + (i - 1) * sec;
			drawRect(r, color);
		}
		r.x = xmid + 2;
		for (int i = 1; i >= 0; i--)
		{
			r.y = ymid + (i - 1) * sec;
			drawRect(r, color);
		}
		r.w = 2;
		r.h = 2 * sec + 2;
		drawRect(r, color);
		r.h = sec + 1;
		r.x += 5;
		drawRect(r, color);
	}
};

int drawPong()
{
	// set up the return value for 2 player or vs cpu, draw the background and border
	int retVal = 0;
	struct Rect r;
	r.h = SCREEN_HEIGHT;
	r.w = SCREEN_WIDTH;
	r.x = r.y = 0;
	uint16 color = makeColor(0x1f, 0, 0);
	drawRect(r, color);
	color = makeColor(0x1f, 0x1f, 0x1f);
	int h = SCREEN_HEIGHT / 2;
	int w = SCREEN_WIDTH / 9;
	int y = h / 4;
	int sub = w / 3;
	r.x = r.y = 0;
	r.h = 1;
	drawRect(r, color);
	r.y = SCREEN_HEIGHT - 1;
	drawRect(r, color);
	r.h = h * 2;
	r.w = 1;
	r.y = 0;
	drawRect(r, color);
	r.x = SCREEN_WIDTH - 1;
	drawRect(r, color);

	//draw p
	r.h = h;
	r.x = w;
	r.y = y;
	r.w = sub;
	drawRect(r, color);
	r.h = sub;
	r.x += sub;
	drawRect(r, color);
	r.x += sub;
	r.h = h / 2 + sub;
	drawRect(r, color);
	r.x -= sub;
	r.y += h / 2;
	r.h = sub;
	drawRect(r, color);
	//draw o
	r.h = h;
	r.x = 3 * w;
	r.y = y;
	r.w = sub;
	drawRect(r, color);
	r.x += (2 * sub);
	drawRect(r, color);
	r.x -= sub;
	r.h = sub;
	drawRect(r, color);
	r.y += h - sub;
	drawRect(r, color);
	//draw n
	r.h = h;
	r.x = 5 * w;
	r.y = y;
	r.w = sub;
	drawRect(r, color);
	r.x += (2 * sub);
	drawRect(r, color);
	r.w = sub;
	r.h = 1;
	int dd = h / sub;
	for (int i = 0; i < h; i++)
	{
		r.y = y + i;
		r.x = (5 * w) + ((2 * i) / dd);
		drawRect(r, color);
	}
	//draw g
	r.h = h;
	r.x = 7 * w;
	r.y = y;
	r.w = sub;
	drawRect(r, color);
	r.h = sub;
	r.x += sub;
	drawRect(r, color);
	r.x += sub;
	r.h = h / 2 - 2 * sub;
	drawRect(r, color);
	r.y += h / 2;
	r.h = h / 2;
	drawRect(r, color);
	r.x -= sub;
	r.y = h + y - sub;
	r.h = sub;
	drawRect(r, color);
	r.y -= h / 2 - sub;
	r.x += ((2 * sub) / 3) - 1;
	drawRect(r, color);
	//draw buttons
	color = makeColor(0x1f, 0x1f, 0x1f);
	r.x = w;
	r.w = 3 * w;
	r.h = y;
	r.y = h + 2 * y;
	drawRect(r, color);
	r.x += 4 * w;
	drawRect(r, color);
	drawButton(r, w, h, y, retVal, 0);
	drawButton(r, w, h, y, !retVal, 1);
	// wait and get input
	while (REG_KEY_INPUT & DOWN)
		if (!((REG_KEY_INPUT)& LEFT) || !((REG_KEY_INPUT)& RIGHT))
		{
			retVal = retVal ? 0 : 1;
			drawButton(r, w, h, y, retVal, 0);
			drawButton(r, w, h, y, !retVal, 1);
			for (int i = 0; i < 20; i++)
				sync();
		}
	r.h = SCREEN_HEIGHT;
	r.w = SCREEN_WIDTH;
	r.x = r.y = 0;
	// revert to black
	color = makeColor(0, 0, 0);
	drawRect(r, color);
	return retVal;
}

void winScreen(int player)
{
	//winScreen
	int retVal = 0;
	struct Rect r;
	r.h = SCREEN_HEIGHT;
	r.w = SCREEN_WIDTH;
	r.x = r.y = 0;
	uint16 color = makeColor(0x1f, 0, 0);
	drawRect(r, color);
	color = makeColor(0x1f, 0x1f, 0x1f);
	int h = SCREEN_HEIGHT / 4;
	int w = SCREEN_WIDTH / 9;
	int y = h / 2;
	int sub = w / 3;
	r.h = 1;
	drawRect(r, color);
	r.y = SCREEN_HEIGHT - 1;
	drawRect(r, color);
	r.h = SCREEN_HEIGHT;
	r.w = 1;
	r.y = 0;
	drawRect(r, color);
	r.x = SCREEN_WIDTH - 1;
	drawRect(r, color);
	// draw p
	r.h = h + sub;
	r.x = 3 * w;
	r.y = y;
	r.w = sub;
	drawRect(r, color);
	r.h = sub;
	r.x += sub;
	drawRect(r, color);
	r.x += sub;
	r.h = h / 2 + sub;
	drawRect(r, color);
	r.x -= sub;
	r.y += h / 2;
	r.h = sub;
	drawRect(r, color);
	// draw w
	r.y = 5 * y;
	r.x = 2 * w;
	r.w = w / 5;
	r.h = h;
	drawRect(r, color);
	r.x += 2 * r.w;
	r.w++;
	drawRect(r, color);
	r.w--;
	r.x += 2 * r.w + 1;
	drawRect(r, color);
	r.x = 2 * w;
	r.y += 2 * y - sub;;
	r.w = w;
	r.h = sub;
	drawRect(r, color);
	// draw i
	r.y = 5 * y;
	r.x = 4 * w + sub;
	r.w = sub;
	r.h = h;
	drawRect(r, color);
	r.x -= sub;
	r.h = sub;
	r.w = w - 2;
	drawRect(r, color);
	r.y += 2 * y - sub;
	drawRect(r, color);
	// draw n
	r.y = 5 * y;
	r.x = 6 * w;
	r.w = sub;
	r.h = h;
	drawRect(r, color);
	r.x += 2 * sub + 2;
	drawRect(r, color);
	r.x = 6 * w;
	r.w = sub;
	r.h = 1;
	int dd = h / sub;
	for (int i = 0; i < h; i++)
	{
		r.y = 5 * y + i;
		r.x = (6 * w) + ((2 * i) / dd);
		drawRect(r, color);
	}
	if (player)
	{
		r.h = h / 2;
		r.x = 5 * w + 2 * sub + 2;
		r.y = y;
		r.w = sub;
		drawRect(r, color);
		r.x -= 2 * sub + 2;
		r.y += r.h;
		drawRect(r, color);
		r.y -= r.h;
		r.h = sub;
		r.w = w;
		for(int i = 0; i < 3; i++)
		{ 
			r.y = y + i * h / 2;
			drawRect(r, color);
		}
	}
	else
	{
		r.h = h + sub;
		r.x = 5 * w + sub;
		r.y = y;
		r.w = sub;
		drawRect(r, color);
	}
	for (int i = 0; i < 240; i++)
		sync();
}

int main()
{
	REG_DISPLAY = VIDEOMODE | BGMODE;
	// initilize the 7 seg
	initSevenSeg();
	// make the used colors
	int yellow = makeColor(0x1f, 0x1f, 0);
	int blue = makeColor(0, 0x0f, 0x1f);
	int white = makeColor(0x1f, 0x1f, 0x1f);
	int black = makeColor(0, 0, 0);
	// make center line
	line.h = 4;
	line.w = 1;
	line.y = 0;
	line.x = SCREEN_WIDTH / 2;
	// make ball and paddle vars, including dimensions and placement
	player.x = player.y = player2.y = 0;
	player.w = player2.w = 8;
	player.h = player2.h = 32;
	player2.x = SCREEN_WIDTH - 8;
	prevPlayer = player;
	prevPlayer2 = player2;
	ball.x = 130;
	ball.y = 80;
	ball.w = ball.h = 8;
	prevBall = ball;
	while (1)
	{
		int top = 0;
		int top2 = 0;
		int ballLeft = 130;
		int ballTop = 80;
		int speedLeft = 1;
		int speedUp = 1;
		int hit = 0;
		int hit2 = 0;
		uint8 score1 = 0;
		uint8 score2 = 0;
		// display title screen menu
		int vsCPU = drawPong();
		//main game loop
		while (1)
		{
			sync();
			// draw over previous frames
			drawRect(prevPlayer, black);
			drawRect(prevPlayer2, black);
			drawRect(prevBall, black);
			// move ball
			ballTop += speedUp;
			ballLeft += speedLeft;
			// get input
			if (!((REG_KEY_INPUT)& DOWN))
				top++;
			if (!((REG_KEY_INPUT)& UP))
				top--;
			if (!((REG_KEY_INPUT)& R) && !vsCPU)
				top2++;
			if (!((REG_KEY_INPUT)& L) && !vsCPU)
				top2--;
			// count down from paddle highlight upon hit
			if (hit > 0)
				hit--;
			if (hit2 > 0)
				hit2--;
			// make sure moving objs stay in bounds
			top = clamp(top, 0, SCREEN_HEIGHT - player.h);
			if (vsCPU && ballLeft + (ball.w / 2) >= 1456 * line.x / 1000)
			{
				if (ballTop + ball.h / 2 < player2.y + player2.h / 2)
					top2 --;
				else
					top2 ++;
			}
			top2 = clamp(top2, 0, SCREEN_HEIGHT - player2.h);
			//super cpu
			//top2 = clamp(ballTop - (player2.h / 2), 0, SCREEN_HEIGHT - player2.h);
			ballTop = clamp(ballTop, 0, SCREEN_HEIGHT - ball.h);
			ballLeft = clamp(ballLeft, 0, SCREEN_WIDTH - ball.w);
			// ball rebound vertical
			if (ballTop == 0 || ballTop == SCREEN_HEIGHT - ball.h)
				speedUp = -speedUp;
			// register hit, score and ball rebound horizontal
			if (ballLeft == 0)
			{
				drawSevenSeg(8, 1, black);
				speedLeft = -speedLeft;
				score2++;
				if (score2 == 10)
					break;
			}
			else if (ballLeft == SCREEN_WIDTH - ball.w)
			{
				drawSevenSeg(8, 0, black);
				speedLeft = -speedLeft;
				score1++;
				if (score1 == 10)
					break;
			}
			else if (hasHit(ballLeft, ballTop, player))
			{
				hit = 60;
				ballLeft = 8;
				speedLeft = -speedLeft;
			}
			else if (hasHit2(ballLeft + ball.w, ballTop, player2))
			{
				hit2 = 60;
				ballLeft = player2.x - ball.w;
				speedLeft = -speedLeft;
			}
			// reassign vars
			player.y = top;
			prevPlayer = player;
			player2.y = top2;
			prevPlayer2 = player2;
			ball.x = ballLeft;
			ball.y = ballTop;
			prevBall = ball;
			// draw paddles and ball
			drawRect(player, hit ? yellow : blue);
			drawRect(player2, hit2 ? yellow : blue);
			drawRect(ball, makeColor(0x1f, 0, 0));
			//draw center line
			for (int i = 0; i < SCREEN_HEIGHT; i++)
				if (i % ball.h == 0)
				{
					line.y = i;
					drawRect(line, white);
				}
			line.y = 0;
			// draw 7 seg
			drawSevenSeg(score1, 0, white);
			drawSevenSeg(score2, 1, white);
		}
		//win screen
		winScreen(score1 == 10 ? 0 : 1);
	}
}
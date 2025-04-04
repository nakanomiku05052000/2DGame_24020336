

#include "stdafx.h"
#include <windows.h>
#include "BaseFunc.h"
#include "BaseObject.h"
#include "Game_Map.h"
#include "MainObject.h"
#include "Timer.h"
#include "ThreatsObject.h"
#include "ExplosionObject.h"
#include "TextObject.h"
#include "PlayerPower.h"
#include "BossObject.h"

BaseObject g_background;
TTF_Font* font_time = NULL;

bool InitData()
{
	bool success = true;
	int ret = SDL_Init(SDL_INIT_VIDEO); 
	if(ret < 0)
		return false;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	g_window = SDL_CreateWindow("Game Cpp SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN); 

	if(g_window == NULL)
	{
		success = false;
	}
	else
	{
		g_screen = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
		if(g_screen == NULL)
			success = false;
		else
		{
			SDL_SetRenderDrawColor(g_screen, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR);
			int imgFlags = IMG_INIT_PNG;
			if(!(IMG_Init(imgFlags) && imgFlags))
				success = false;
		}

		if(TTF_Init() == -1)
		{
			success = false;
		}

		font_time = TTF_OpenFont("font//dlxfont.ttf", 15);
		if(font_time == NULL)
		{
			success = false;
		}
	}

	return success;
}

bool LoadBackground()
{
	bool ret = g_background.LoadImg("img//background.png", g_screen);
	if(ret == false)
		return false;

	return true;
}

void close()
{
	g_background.Free();
	SDL_DestroyRenderer(g_screen);
	g_screen = NULL;

	SDL_DestroyWindow(g_window);
	g_window = NULL;

	IMG_Quit();
	SDL_Quit();
}

std::vector<ThreatsObject*> MakeThreatList()
{
	std::vector<ThreatsObject*> list_threats;

	ThreatsObject* dynamic_threats = new ThreatsObject[20];
	 
	for(int i = 0; i < 20; i++)
	{
		ThreatsObject* p_threat = (dynamic_threats + i);
		if(p_threat != NULL)
		{
			p_threat->LoadImg("img//threat_left.png", g_screen);
			p_threat->set_clips();
			p_threat->set_type_move(ThreatsObject::MOVE_IN_SPACE_THREAT);
			p_threat->set_x_pos(650 + i*1200);
			p_threat->set_y_pos(200);

			int pos1 = p_threat->get_x_pos() - 60;
			int pos2 = p_threat->get_x_pos() + 60;
			p_threat->SetAnimationPos(pos1, pos2);
			p_threat->set_input_left(1);
			list_threats.push_back(p_threat);
		}
	}

	ThreatsObject* threats_objects = new ThreatsObject[20];

	for(int i = 0; i < 20; i++)
	{
		ThreatsObject* p_threat = (threats_objects + i);
		if(p_threat != NULL)
		{
			p_threat->LoadImg("img//threat.png", g_screen);
			p_threat->set_clips();
			p_threat->set_x_pos(700 + i * 1200);
			p_threat->set_y_pos(250);
			p_threat->set_type_move(ThreatsObject::STATIC_THREAT);
			p_threat->set_input_left(0);

			BulletObject* p_bullet = new BulletObject();
			p_threat->InitBullet(p_bullet, g_screen);
			list_threats.push_back(p_threat);
		}
	}

	return list_threats; 
}

int main(int argc, char* agrv[])
{
	Timer fps_timer;

	if(InitData() == false)
		return -1;

	if(LoadBackground() == false)
		return -1;
	
	GameMap game_map;
	game_map.LoadMap("map/map01.dat");
	game_map.LoadTiles(g_screen);

	MainObject p_player;
	p_player.LoadImg("img//player_right.png", g_screen);
	p_player.set_clips();

	PlayerPower player_power;
	player_power.Init(g_screen);

	PLayerMoney player_money;
	player_money.Init(g_screen);
	player_money.SetPos(SCREEN_WIDTH*0.5 - 300, 8);

	std::vector<ThreatsObject*> threats_list = MakeThreatList();

	//BOSS THREAT
	BossObject bossObject;
	bool ret = bossObject.LoadImg("img//boss_object.png", g_screen);
	bossObject.set_clips();
	int xPosBoss = MAX_MAP_X*TILE_SIZE - SCREEN_WIDTH*0.6;
	bossObject.set_xpos(xPosBoss);
	bossObject.set_ypos(10); 

	ExplosionObject exp_threat;
	bool tRet = exp_threat.LoadImg("img//exp3.png", g_screen);
	if(!tRet) return -1;
	exp_threat.set_clip();

	ExplosionObject exp_main;
	bool mRet = exp_main.LoadImg("img//exp3.png", g_screen);
	exp_main.set_clip();
	if(!mRet) return -1;

	int num_die = 0;

	// Time text
	TextObject time_game;
	time_game.SetColor(TextObject::BLACK_TEXT);

	TextObject point_game;
	point_game.SetColor(TextObject::BLACK_TEXT);
	UINT point_value = 0;

	TextObject money_game;
	money_game.SetColor(TextObject::BLACK_TEXT);

	bool is_quit = false;
	while(!is_quit)
	{
		fps_timer.start();
		while(SDL_PollEvent(&g_event) != 0)
		{
			if(g_event.type == SDL_QUIT)
			{
				is_quit = true;
			}

			p_player.HandleInputAct(g_event, g_screen);
		}

		SDL_SetRenderDrawColor(g_screen, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR, RENDER_DRAW_COLOR);
		SDL_RenderClear(g_screen);

		g_background.Render(g_screen, NULL);

		Map map_data = game_map.getMap();

		p_player.HandleBullet(g_screen);
		p_player.SetMapXY(map_data.start_x_, map_data.start_y_);
		p_player.DoPlayer(map_data); 
		p_player.Show(g_screen);

		game_map.SetMap(map_data);
		game_map.DrawMap(g_screen);

		player_power.Show(g_screen);
		player_money.Show(g_screen);

		for(int i = 0; i < threats_list.size(); i++)
		{
			ThreatsObject* p_threat = threats_list.at(i);
			if(p_threat != NULL)
			{
				p_threat->SetMapXY(map_data.start_x_, map_data.start_y_);
				p_threat->ImpMoveType(g_screen);
				p_threat->DoPlayer(map_data);
				p_threat->MakeBullet(g_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
				p_threat->Show(g_screen);

				SDL_Rect rect_player = p_player.GetRectFrame();
				bool bCol1 = false;
				std::vector<BulletObject*> tBullet_list  = p_threat->get_bullet_list();
				for(int jj = 0; jj < tBullet_list.size(); jj++)
				{
					BulletObject* pt_bullet = tBullet_list.at(jj);
					if(pt_bullet)
					{
						bCol1 = SDLBaseFunc::CheckCollision(pt_bullet->GetRect(), rect_player);
						if(bCol1 == true)
						{
							p_threat->RemoveBullet(jj);
							break;
						}
					}
				}

				SDL_Rect rect_threat = p_threat->GetRectFrame();
				bool bCol2 = SDLBaseFunc::CheckCollision(rect_player, rect_threat);

				if(bCol1 || bCol2)
				{
					int width_exp_frame = exp_main.get_frame_width();
					int height_exp_frame = exp_main.get_frame_height();
					for(int ex = 0; ex < 4; ex++)
					{
						int x_pos = (p_player.GetRect().x + p_player.get_frame_width() * 0.5) - width_exp_frame*0.5;
						int y_pos = (p_player.GetRect().y + p_player.get_frame_height() * 0.5) - height_exp_frame*0.5;

						exp_main.set_frame(ex);
						exp_main.SetRect(x_pos, y_pos);
						exp_main.Show(g_screen);
						SDL_RenderPresent(g_screen);
					}

					num_die++;
					if(num_die <= 3)
					{
						p_player.SetRect(0, 0);
						p_player.set_rerun_time(60);
						SDL_Delay(1000);
						player_power.Decrease();
						player_power.Render(g_screen);
						continue;
					}
					else
					{
						if(MessageBox(NULL, L"GAME OVER!!!", L"Info", MB_OK | MB_ICONSTOP) == IDOK)
						{
							p_threat->Free();
							close();     
							SDL_Quit();
							return 0;
						}
					}
				}
			}
		}

		int frame_exp_width = exp_threat.get_frame_width();
		int frame_exp_height = exp_threat.get_frame_height();

		std::vector<BulletObject*> bullet_arr = p_player.get_bullet_list();
		for(int bl = 0; bl < bullet_arr.size(); bl++)
		{
			BulletObject* p_bullet = bullet_arr.at(bl);
			if(p_bullet != NULL)
			{
				for(int t = 0; t < threats_list.size(); t++)
				{
					ThreatsObject* obj_threat = threats_list.at(t);
					if(obj_threat != NULL)
					{
						SDL_Rect tRect;
						tRect.x = obj_threat->GetRect().x;
						tRect.y = obj_threat->GetRect().y;
						tRect.w = obj_threat->get_width_frame();
						tRect.h = obj_threat->get_height_frame();

						SDL_Rect bRect = p_bullet->GetRect();

						bool bCol = SDLBaseFunc::CheckCollision(bRect, tRect);

						if(bCol)
						{
							point_value++;
							for(int ex = 0; ex < NUM_FRAME_EXP; ex++)
							{
								int x_pos = p_bullet->GetRect().x - frame_exp_width * 0.5;
								int y_pos = p_bullet->GetRect().y - frame_exp_height * 0.5;

								exp_threat.set_frame(ex);
								exp_threat.SetRect(x_pos, y_pos);
								exp_threat.Show(g_screen);
							}

							p_player.RemoveBullet(bl);
							obj_threat->Free();
							threats_list.erase(threats_list.begin() + t);
						}
					}
				}
			}
		}

		// Show game time
		std::string str_time = "Time: ";
		Uint32 time_val = SDL_GetTicks() / 1000;
		Uint32 val_time = 300 - time_val;
		if(val_time <= 0)
		{
			if(MessageBox(NULL, L"GAME OVER!!!", L"Info", MB_OK | MB_ICONSTOP) == IDOK)
			{
				is_quit = true;
				break;
			}	
		}
		else
		{
			std::string str_val = std::to_string(val_time);
			str_time += str_val;

			time_game.SetText(str_time);
			time_game.LoadFromRenderText(font_time, g_screen);
			time_game.RenderText(g_screen, SCREEN_WIDTH - 200, 15);
		}

		std::string val_str_point = std::to_string(point_value);
		std::string strPoint("Point: ");
		strPoint +=val_str_point;

		point_game.SetText(strPoint);
		point_game.LoadFromRenderText(font_time, g_screen);
		point_game.RenderText(g_screen, SCREEN_WIDTH*0.5 - 50, 15);

		int money_value = p_player.GetMoneyCount();
		std::string money_str = std::to_string(money_value);

		money_game.SetText(money_str);
		money_game.LoadFromRenderText(font_time, g_screen);
		money_game.RenderText(g_screen, SCREEN_WIDTH*0.5 - 250, 15);

		// Show Boss
		int val = MAX_MAP_X*TILE_SIZE - (map_data.start_x_ + p_player.GetRect().x);
		if(val <= SCREEN_WIDTH)
		{
			bossObject.SetMapXY(map_data.start_x_, map_data.start_y_);
			bossObject.DoPlayer(map_data);
			bossObject.MakeBullet(g_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
			bossObject.Show(g_screen);
		}

		SDL_RenderPresent(g_screen);

		int real_time = fps_timer.get_ticks();
		int time_on_frame = 1000 / fps;			//ms

		if(real_time < time_on_frame)
		{
			int delay_time = time_on_frame - real_time;
			if(delay_time >= 0)
				SDL_Delay(delay_time);
		}
	}

	for(int i = 0; i < threats_list.size(); i++)
	{
		ThreatsObject* p_threat = threats_list.at(i);
		if(p_threat != NULL)
		{
			p_threat->Free();
			p_threat = NULL;
		}
	}

	threats_list.clear();

	close();
	return 0;	
}
//#########################
//Pekka Kana 2 Level Editor
//by Janne Kivilahti from Piste Gamez
//---------------
//Pekka Kana 2 Level Editor main code
//
//This code contains all PK2 LE logic
//It uses the map.cpp and sprite.cpp
//to draw the maps and sprites
//---------------
//TODO:
// -Test map directly
// -Transparent foreground when edit background
// -Don't use absolute paths
//#########################
//Code for gdb:
//"gdb break Level_Editor_Menu_Log()"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <unistd.h>
#include <dirent.h>

#include "PisteEngine.h"
#include "map.h"
#include "sprite.h"

#define GAME_NAME "PK2 Level Editor"
#define _MAX_DIR 100

using namespace std;

//##################################################
//Constants
//##################################################

const int SCREEN_WIDTH			= 1024;
const int SCREEN_HEIGHT			= 768;

const UCHAR MENU_EI				= 100;
const UCHAR	MENU_KARTTA			= 0;
const UCHAR	MENU_PALIKAT		= 1;
const UCHAR	MENU_INFO			= 2;
const UCHAR	MENU_SPRITET		= 3;
const UCHAR	MENU_TIEDOSTOT		= 4;
const UCHAR	MENU_SAVE			= 5;
const UCHAR	MENU_HELP			= 6;
const UCHAR	MENU_EXIT			= 7;
const UCHAR	MENU_TOOLS			= 8;
const UCHAR	MENU_LOKI			= 9;

const UCHAR N_OF_MENUS			= 10;

const UCHAR EDIT_MAP			= 0;
const UCHAR EDIT_MENU			= 1;
const UCHAR EDIT_SPRITE			= 2;

const int MAX_DIR_FILES			= 1000;

const int FILE_DIR				= 1;
const int FILE_MAP				= 2;
const int FILE_BMP				= 3;
const int FILE_SPR				= 4;
const int FILE_MUS				= 5;

const int EDIT_BACKGROUND		= 0;
const int EDIT_WALLS			= 1;
const int EDIT_JUST_BACKGROUND 	= 2;

const int MAX_SPRITES			= 600;

const int KENTTA_FILE			= 1;
const int KENTTA_NIMI			= 2;
const int KENTTA_TEKIJA			= 3;
const int KENTTA_MUSIIKKI		= 4;

const int MAX_LOG_LABEL			= 55;
const UCHAR LOG_INFO			= 1;
const UCHAR LOG_VIRHE			= 2;


//##################################################
//Structs
//##################################################

struct TOOL_LIST
{
	bool		nakyva;
	int			x,y,
				valinta;
};

struct MENU
{
	char		otsikko[30];
	int			x;
	int			y;
	int			leveys;
	int			korkeus;
	bool		piilota;
	int			pika;
	TOOL_LIST	lista;
};

struct LEFILE
{
	char		name[_MAX_PATH];
	UCHAR		type;
};

struct LEIKEPOYTA
{
	UCHAR	taustat[PK2KARTTA_KARTTA_KOKO];
	UCHAR	seinat[PK2KARTTA_KARTTA_KOKO];
	UCHAR	spritet[PK2KARTTA_KARTTA_KOKO];
	RECT	alue;
};

struct LOKIMERKINTA
{
	char	teksti[200];
	UCHAR	type;
};

struct ASETUKSET
{
	char		tyohakemisto[_MAX_PATH];
	char		epd_path[_MAX_PATH];
	char		editor_path[_MAX_PATH];
	char		bg_path[_MAX_PATH];
	char		tile_path[_MAX_PATH];
	char		mapfile[_MAX_PATH];
	MENU		menut[N_OF_MENUS];
	int			kartta_x;
	int			kartta_y;
};

//##################################################
//Global Variables
//##################################################

bool Error				= false;// jos tämä muuttuu todeksi niin ohjelma lopetetaan
char virheviesti[60];		

bool running				= true;// onko ikkuna kiinni

double cos_table[360];
double sin_table[360];
int degree = 0;

UCHAR edit_screen = EDIT_MAP;
UCHAR active_menu = MENU_EI;

bool moving_window = false;
int  virt_x;
int  virt_y;

bool editing_text = false;

char viesti[60];
char tyohakemisto[_MAX_PATH];
char pk2_path[_MAX_PATH] = " ";
char epd_path[_MAX_PATH] = " ";
char editor_path[_MAX_PATH] = "";
char bg_path[_MAX_PATH] = " ";
char tile_path[_MAX_PATH] = " ";
LEFILE tiedostot[MAX_DIR_FILES];

char mapfile[_MAX_PATH];

DWORD tiedostoja = 0;

bool nayta_hakemistot = true;

PK2Kartta *kartta, *undo;
bool varmistettu = false;

RECT kartta_kohde = {0,0,0,0};
int	 kartta_kohde_status = 0;
RECT kartta_lahde = {0,0,0,0};

//KUVABUFFERIT
int  kuva_editori;
int  kuva_kartta;

//FONTIT
int fontti1;
int fontti2;

//KONTROLLIT
int mouse_x = 10;
int mouse_y = 10;
int key_delay = 0;

//MENUT
MENU menut[N_OF_MENUS];
int menu_palikat_nayta_y = 0;
int menu_tiedostot_nayta = 1;
int menu_tiedostot_eka = 0;
int menu_spritet_eka = 0;

//KARTTA
int kartta_x = 0;
int kartta_y = 0;
int edit_palikka = 0;
int edit_kerros = EDIT_WALLS;

int focustile_etu = 255;
int focustile_taka = 255;
int focussprite = 255;

bool kartta_ladattu = false;

int palikka_animaatio = 0;

bool animaatio_paalla = true;

bool show_sprites = true;

LEIKEPOYTA leikepoyta;
bool aseta_leikepoyta_alue = false;
RECT leikepoyta_alue = {0,0,0,0};
TOOL_LIST leikepoyta_lista;

PK2Sprite_Prototyyppi protot[MAX_PROTOTYYPPEJA];
int seuraava_vapaa_proto = 0;
//int vakio_spriteja = 0;
//PK2Sprite spritet[MAX_SPRITES];
int spriteja = 0;

int proto_valittu = MAX_PROTOTYYPPEJA;

//TEKSTINKÄSITTELY
int editKenttaId = 0;
char editTeksti[500];
int editKursori = 0;
char editMerkki = ' ';


//LOKI
LOKIMERKINTA loki[MAX_LOG_LABEL];

bool kirjoita_loki = false;

UCHAR karttavarit[150];

//ASETUKSET
ASETUKSET asetukset;

bool exit_editor = false;
bool piirto_aloitettu = false;

//##################################################
//Prototypes
//##################################################

int Level_Editor_Log_Save(char *viesti);
int Level_Editor_Draw_Nelio(int vasen, int yla, int oikea, int ala, UCHAR color);
int Level_Editor_Map_Update();
void Level_Editor_Laske_TileVarit();
void Level_Editor_Aseta_Palikat(char *filename);
void Level_Editor_Aseta_Taustakuva(char *filename);
int Level_Editor_Map_Defaults();
int Level_Editor_Kartta_Lataa(char *filename);

//##################################################
//Functions
//##################################################

void Level_Editor_Log_Start(){
	for (int i=0;i<MAX_LOG_LABEL;i++){
		loki[i].type = 0;
		strcpy(loki[i].teksti," ");
	}
}

void Level_Editor_Log_Write(char *teksti, UCHAR tyyppi){
	strcpy(viesti,teksti);
	
	for (int i=0;i<MAX_LOG_LABEL-1;i++){
		strcpy(loki[i].teksti,loki[i+1].teksti);
		loki[i].type = loki[i+1].type;

	}
	loki[MAX_LOG_LABEL-1].type = tyyppi;
	strcpy(loki[MAX_LOG_LABEL-1].teksti,teksti);
}

int Level_Editor_Get_Settings(){
	int i;

	char	tiedostonimi[_MAX_PATH];

	strcpy(tiedostonimi,pk2_path);
	strcat(tiedostonimi,"/editor/settings.dat");

	Level_Editor_Log_Write("loading settings...", LOG_INFO);

	ifstream *tiedosto = new ifstream(tiedostonimi, ios::in);

	if (tiedosto->fail()){
		delete (tiedosto);
		Level_Editor_Log_Write("no settings saved.", LOG_INFO);
		return 1;
	}
	
	tiedosto->read((char *)&asetukset,sizeof(asetukset));
	
	if (tiedosto->fail()){
		delete (tiedosto);
		strcpy(viesti,"could not read settings.dat");
		Level_Editor_Log_Write("loading settings failed.", LOG_VIRHE);
		return 1;
	}

	delete (tiedosto);

	strcpy(bg_path,asetukset.bg_path);
	strcpy(epd_path,asetukset.epd_path);
	strcpy(mapfile,asetukset.mapfile);
	strcpy(tile_path,asetukset.tile_path);
	strcpy(tyohakemisto,asetukset.tyohakemisto);

	for (i=0;i<N_OF_MENUS;i++){
		menut[i].x = asetukset.menut[i].x;
		menut[i].y = asetukset.menut[i].y;
		menut[i].piilota = asetukset.menut[i].piilota;
	}	

	kartta_x = asetukset.kartta_x;
	kartta_y = asetukset.kartta_y;

	if (chdir(tyohakemisto) == 0)
		getcwd(tyohakemisto, _MAX_PATH );

	//Level_Editor_Kartta_Lataa(mapfile);//TODO
	char hake[_MAX_PATH] = "";
	strcpy(hake,pk2_path);
	strcat(hake,"/episodes/rooster island 2/level4.map");
	Level_Editor_Kartta_Lataa(hake);
	
	mouse_x = 250;
	mouse_y = 250;

	return 0;
}

void Level_Editor_Save_Settings(){
	int i;
	
	Level_Editor_Log_Write("saving settings", LOG_INFO);

	strcpy(asetukset.bg_path,bg_path);
	strcpy(asetukset.editor_path,bg_path);
	strcpy(asetukset.epd_path,epd_path);
	strcpy(asetukset.mapfile,mapfile);
	strcpy(asetukset.tile_path,tile_path);
	strcpy(asetukset.tyohakemisto,tyohakemisto);

	for (i=0;i<N_OF_MENUS;i++){
		asetukset.menut[i].x = menut[i].x;
		asetukset.menut[i].y = menut[i].y;
		asetukset.menut[i].piilota = menut[i].piilota;
	}

	asetukset.menut[MENU_EXIT].piilota = true;

	asetukset.kartta_x = kartta_x;
	asetukset.kartta_y = kartta_y;

	char	tiedostonimi[_MAX_PATH];

	strcpy(tiedostonimi,pk2_path);
	strcat(tiedostonimi,"/editor/settings.dat");

	ofstream *tiedosto = new ofstream(tiedostonimi, ios::binary);
	tiedosto->write ((char *)&asetukset, sizeof (asetukset));
	
	delete (tiedosto);
}

//##################################################
//Clipboard
//##################################################

void Level_Editor_Leikepoyta_Alusta(){
	memset(leikepoyta.seinat, 0,sizeof(leikepoyta.seinat));
	memset(leikepoyta.taustat,0,sizeof(leikepoyta.taustat));
	memset(leikepoyta.spritet,0,sizeof(leikepoyta.spritet));

	leikepoyta.alue.left	= 0;
	leikepoyta.alue.right	= 0;
	leikepoyta.alue.top		= 0;
	leikepoyta.alue.bottom	= 0;
}

void Level_Editor_Leikepoyta_Kopioi(RECT alue){
	Level_Editor_Leikepoyta_Alusta();

	if (alue.right > PK2KARTTA_KARTTA_LEVEYS)
		alue.right = PK2KARTTA_KARTTA_LEVEYS;
	
	if (alue.left < 0)
		alue.left = 0;

	if (alue.bottom > PK2KARTTA_KARTTA_KORKEUS)
		alue.bottom = PK2KARTTA_KARTTA_KORKEUS;

	if (alue.top < 0)
		alue.top = 0;	

	leikepoyta.alue.left = 0;
	leikepoyta.alue.top = 0;
	leikepoyta.alue.right = alue.right - alue.left;
	leikepoyta.alue.bottom = alue.bottom - alue.top;

	for (int x=alue.left;x<alue.right;x++)
		for (int y=alue.top;y<alue.bottom;y++){
			leikepoyta.taustat[x-alue.left+(y-alue.top)*PK2KARTTA_KARTTA_LEVEYS] = kartta->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS];
			leikepoyta.seinat[ x-alue.left+(y-alue.top)*PK2KARTTA_KARTTA_LEVEYS] = kartta->seinat[ x+y*PK2KARTTA_KARTTA_LEVEYS];
			leikepoyta.spritet[x-alue.left+(y-alue.top)*PK2KARTTA_KARTTA_LEVEYS] = kartta->spritet[x+y*PK2KARTTA_KARTTA_LEVEYS];
		}
	
	Level_Editor_Map_Update();
}

void Level_Editor_Leikepoyta_Liita(RECT alue){
	if (alue.right > PK2KARTTA_KARTTA_LEVEYS)
		alue.right = PK2KARTTA_KARTTA_LEVEYS;
	
	if (alue.left < 0)
		alue.left = 0;

	if (alue.bottom > PK2KARTTA_KARTTA_KORKEUS)
		alue.bottom = PK2KARTTA_KARTTA_KORKEUS;

	if (alue.top < 0)
		alue.top = 0;

	int lp_leveys  = leikepoyta.alue.right - leikepoyta.alue.left,
		lp_korkeus = leikepoyta.alue.bottom - leikepoyta.alue.top;

	if (lp_leveys > 0 && lp_korkeus > 0){
		for (int x=alue.left;x<alue.right;x++)
			for (int y=alue.top;y<alue.bottom;y++){
				//kartta->spritet[x+y*PK2KARTTA_KARTTA_LEVEYS] = leikepoyta.spritet[(x-alue.left)%lp_leveys +((y-alue.top)%lp_korkeus)*PK2KARTTA_KARTTA_LEVEYS];
				if (edit_kerros == EDIT_BACKGROUND || edit_kerros == EDIT_WALLS)
					kartta->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS] = leikepoyta.taustat[(x-alue.left)%lp_leveys +((y-alue.top)%lp_korkeus)*PK2KARTTA_KARTTA_LEVEYS];
				if (edit_kerros == EDIT_WALLS || edit_kerros == EDIT_JUST_BACKGROUND)
					kartta->seinat[ x+y*PK2KARTTA_KARTTA_LEVEYS] = leikepoyta.seinat[ (x-alue.left)%lp_leveys +((y-alue.top)%lp_korkeus)*PK2KARTTA_KARTTA_LEVEYS];
			}	
	}

	Level_Editor_Map_Update();
}

void Level_Editor_Leikepoyta_Liita_Koko(RECT alue){

	alue.right  = alue.left + leikepoyta.alue.right  - leikepoyta.alue.left;
	alue.bottom = alue.top  + leikepoyta.alue.bottom - leikepoyta.alue.top;

	if (alue.right > PK2KARTTA_KARTTA_LEVEYS)
		alue.right = PK2KARTTA_KARTTA_LEVEYS;
	
	if (alue.left < 0)
		alue.left = 0;

	if (alue.bottom > PK2KARTTA_KARTTA_KORKEUS)
		alue.bottom = PK2KARTTA_KARTTA_KORKEUS;

	if (alue.top < 0)
		alue.top = 0;

	int lp_leveys  = leikepoyta.alue.right - leikepoyta.alue.left,
		lp_korkeus = leikepoyta.alue.bottom - leikepoyta.alue.top;

	if (lp_leveys > 0 && lp_korkeus > 0){
		for (int x=alue.left;x<alue.right;x++)
			for (int y=alue.top;y<alue.bottom;y++){
				if (edit_kerros == EDIT_BACKGROUND || edit_kerros == EDIT_WALLS)
					kartta->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS] = leikepoyta.taustat[(x-alue.left) +(y-alue.top)*PK2KARTTA_KARTTA_LEVEYS];
				if (edit_kerros == EDIT_WALLS || edit_kerros == EDIT_JUST_BACKGROUND)
					kartta->seinat[ x+y*PK2KARTTA_KARTTA_LEVEYS] = leikepoyta.seinat[ (x-alue.left) +(y-alue.top)*PK2KARTTA_KARTTA_LEVEYS];
			}	
	}

	Level_Editor_Map_Update();
}

void Level_Editor_Leikepoyta_Leikkaa(RECT alue){
	Level_Editor_Leikepoyta_Alusta();

	if (alue.right > PK2KARTTA_KARTTA_LEVEYS)
		alue.right = PK2KARTTA_KARTTA_LEVEYS;
	
	if (alue.left < 0)
		alue.left = 0;

	if (alue.bottom > PK2KARTTA_KARTTA_KORKEUS)
		alue.bottom = PK2KARTTA_KARTTA_KORKEUS;

	if (alue.top < 0)
		alue.top = 0;	

	leikepoyta.alue.left = 0;
	leikepoyta.alue.top = 0;
	leikepoyta.alue.right = alue.right - alue.left;
	leikepoyta.alue.bottom = alue.bottom - alue.top;

	for (int x=alue.left;x<alue.right;x++)
		for (int y=alue.top;y<alue.bottom;y++){
			if (edit_kerros == EDIT_BACKGROUND || edit_kerros == EDIT_WALLS){
				leikepoyta.taustat[x-alue.left+(y-alue.top)*PK2KARTTA_KARTTA_LEVEYS] = kartta->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS];
				kartta->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS] = 255;
			}
			if (edit_kerros == EDIT_WALLS || edit_kerros == EDIT_JUST_BACKGROUND){
				leikepoyta.seinat[ x-alue.left+(y-alue.top)*PK2KARTTA_KARTTA_LEVEYS] = kartta->seinat[ x+y*PK2KARTTA_KARTTA_LEVEYS];
				kartta->seinat[ x+y*PK2KARTTA_KARTTA_LEVEYS] = 255;
			}
		}

	Level_Editor_Map_Update();
}

void Level_Editor_Leikepoyta_Piirra(){
	Level_Editor_Draw_Nelio((leikepoyta_alue.left-kartta_x) *32-1,(leikepoyta_alue.top-kartta_y)  *32-1,
							  (leikepoyta_alue.right-kartta_x)*32+1,(leikepoyta_alue.bottom-kartta_y)*32+1,0);

	Level_Editor_Draw_Nelio((leikepoyta_alue.left-kartta_x) *32,(leikepoyta_alue.top-kartta_y)  *32,
							  (leikepoyta_alue.right-kartta_x)*32,(leikepoyta_alue.bottom-kartta_y)*32,31);

	Level_Editor_Draw_Nelio((leikepoyta_alue.left-kartta_x) *32+1,(leikepoyta_alue.top-kartta_y)  *32+1,
							  (leikepoyta_alue.right-kartta_x)*32-1,(leikepoyta_alue.bottom-kartta_y)*32-1,0);
}

//Unused
int Level_Editor_Set_Episode_Directory(){
	getcwd(epd_path, _MAX_PATH );
	return 0;
}

int Level_Editor_Start_PK2_Directory(){
	strcpy(PK2Kartta::pk2_hakemisto, pk2_path);
	strcat(PK2Kartta::pk2_hakemisto, "/");

	strcpy(bg_path, PK2Kartta::pk2_hakemisto);
	strcat(bg_path, "/gfx/scenery/");

	strcpy(tile_path, PK2Kartta::pk2_hakemisto);
	strcat(tile_path, "/gfx/tiles/");

	return 0;
}

//Unused
int Level_Editor_Tallenna_pk2_path(char *hakemisto){
	char tal[_MAX_DIR] = "";
	strcpy(tal,editor_path);
	strcat(tal,"//pk2le_dir.ini");
	Level_Editor_Log_Write("saving pk2le_dir.ini.", LOG_INFO);

	if (strcmp(pk2_path," ")==0){
		ofstream *tiedosto = new ofstream(tal, ios::out);
		tiedosto->write(hakemisto, 255);
		
		tiedosto->close();

		if (tiedosto->fail()){
			delete (tiedosto);
			Level_Editor_Log_Write("failed to save pk2_dir.ini.", LOG_VIRHE);
			return 1;
		}

		delete (tiedosto);		
		strcpy(pk2_path, hakemisto);

		strcpy(PK2Kartta::pk2_hakemisto, hakemisto);
		strcat(PK2Kartta::pk2_hakemisto, "/");

		strcpy(bg_path, PK2Kartta::pk2_hakemisto);
		strcat(bg_path, "/gfx/scenery/");

		strcpy(tile_path, PK2Kartta::pk2_hakemisto);
		strcat(tile_path, "/gfx/tiles/");

		if (chdir(pk2_path) == 0)
			getcwd(tyohakemisto, _MAX_PATH );		
		
		//Level_Editor_Start_PK2_Directory();
		strcpy(viesti,"pk2 directory found");

		Level_Editor_Map_Defaults();
		menut[MENU_PALIKAT].piilota = false;
		menut[MENU_SPRITET].piilota = false;
	}

	return 0;
}

void Level_Editor_Calculate_Sprites(){
	int proto;

	spriteja = 0;

	for (int x=0;x<PK2KARTTA_KARTTA_KOKO;x++){
		proto = kartta->spritet[x];
		if (proto != 255)
			spriteja++;
	}
	
	Level_Editor_Log_Save("Spritejen maara laskettu. \n");
}

void Level_Editor_Remove_Sprite(int proto){

	if (strcmp(protot[proto].nimi,"")!=0){
		Level_Editor_Log_Save("Poistetaan proto ");
		Level_Editor_Log_Save(protot[proto].nimi);
		Level_Editor_Log_Save("\n");
	}
	
	protot[proto].Uusi();
	strcpy(kartta->protot[proto],"");

	if (seuraava_vapaa_proto > 0)
		seuraava_vapaa_proto--;

	if (kartta->pelaaja_sprite > proto)
		kartta->pelaaja_sprite--;

	if (proto < MAX_PROTOTYYPPEJA-1){
		for (int i=proto;i<MAX_PROTOTYYPPEJA;i++){
			strcpy(kartta->protot[i],kartta->protot[i+1]);
			protot[i].Kopioi(protot[i+1]);
		}

		strcpy(kartta->protot[MAX_PROTOTYYPPEJA-1],"");
		protot[MAX_PROTOTYYPPEJA-1].Uusi();

		for (DWORD x=0; x<PK2KARTTA_KARTTA_KOKO; x++){
			if (kartta->spritet[x] == proto)
				kartta->spritet[x] = 255;

			if (kartta->spritet[x] > proto && kartta->spritet[x] != 255)
				kartta->spritet[x]--;
		}
	}

	int i = MAX_PROTOTYYPPEJA-1;
	while (strcmp(protot[i].nimi,"")==0 && i >= 0)
		i--;

	if (i!=-1)
		seuraava_vapaa_proto = i+1;
}

void Level_Editor_Remove_Empty_Sprites(){
	Level_Editor_Log_Write("deleting empty sprites.", LOG_INFO);
	for (int i=0; i<MAX_PROTOTYYPPEJA; i++)
		if (strcmp(protot[i].nimi,"")==0)
			Level_Editor_Remove_Sprite(i);
}

void Level_Editor_Remove_All_Sprites(){
	Level_Editor_Log_Write("deleting all sprites.", LOG_INFO);
	//Level_Editor_Log_Save("Tyhjennetaan kaikki prototyypit.\n");

	for (int i=0; i<MAX_PROTOTYYPPEJA; i++){
		Level_Editor_Log_Save("Poistetaan proto ");
		Level_Editor_Log_Save(protot[i].nimi);
		Level_Editor_Log_Save("\n");
		//Level_Editor_Remove_Sprite(i);
		protot[i].Uusi();
		strcpy(kartta->protot[i],"");
	}

	seuraava_vapaa_proto = 0;
}

int Level_Editor_Count_Sprite(int proto) {
	int lkm = 0;

	for (DWORD x=0; x<PK2KARTTA_KARTTA_KOKO; x++)
		if (kartta->spritet[x] == proto)
			lkm++;

	return lkm;
}

void Level_Editor_Delete_Unused_Sprites(){
	int i = 0;

	Level_Editor_Log_Write("deleting unused sprites:", LOG_INFO);
	//Level_Editor_Log_Save("Tyhjennetaan kaikki prototyypit.\n");

	//for (int i=0; i<MAX_PROTOTYYPPEJA; i++)	{
	while(i < MAX_PROTOTYYPPEJA){
		if (strcmp(protot[i].nimi,"")!=0) {
			if (Level_Editor_Count_Sprite(i) == 0 && i != kartta->pelaaja_sprite){
				Level_Editor_Log_Save("Poistetaan proto ");
				Level_Editor_Log_Save(protot[i].nimi);
				Level_Editor_Log_Save("\n");
				Level_Editor_Log_Write(protot[i].nimi, LOG_INFO);
				Level_Editor_Remove_Sprite(i);
			}
			else
				i++;
		}
		else
			i++;
	}

	seuraava_vapaa_proto = 0;
}

// Ladataan kartan mukana tulevat spritet
int Level_Editor_Sprite_File_Load(char *polku, char *tiedosto){
	char msg[200];
	strcpy(msg,"loading sprite: ");
	strcat(msg,tiedosto);
	Level_Editor_Log_Write(msg, LOG_INFO);
		
	//Level_Editor_Log_Write("loading sprite.", LOG_INFO);
	Level_Editor_Log_Save("Ladataan vanha prototyyppi ");
	Level_Editor_Log_Save(tiedosto);	
	Level_Editor_Log_Save("\n");	
	
	char tiedostopolku[255];

	strcpy(tiedostopolku,polku);
	strcat(tiedostopolku,"/");	

	
	if (seuraava_vapaa_proto < MAX_PROTOTYYPPEJA){
		if (protot[seuraava_vapaa_proto].Lataa(tiedostopolku, tiedosto) == 1){
			strcpy(viesti,"could not load sprite ");
			strcat(viesti,tiedosto);
			Level_Editor_Log_Write(viesti, LOG_VIRHE);

			protot[seuraava_vapaa_proto].Uusi();
			strcpy(protot[seuraava_vapaa_proto].nimi,"loading error!");
			strcpy(protot[seuraava_vapaa_proto].tiedosto,tiedosto);
			protot[seuraava_vapaa_proto].korkeus = 10;
			protot[seuraava_vapaa_proto].leveys  = 10;
			protot[seuraava_vapaa_proto].kuva_frame_leveys = 10;
			protot[seuraava_vapaa_proto].kuva_frame_korkeus = 10;

			seuraava_vapaa_proto++;

			return 1;
		}
	}
	else
		return 1;

	seuraava_vapaa_proto++;

	return 0;
}

int Level_Editor_Load_New_Sprite(char *polku, char *tiedosto){
	char msg[200];
	strcpy(msg,"loading new sprite: ");
	//strcat(msg,polku);
	//strcat(msg,"/");
	strcat(msg,tiedosto);
	Level_Editor_Log_Write(msg, LOG_INFO);
	
	//Level_Editor_Log_Write("loading new sprite.", LOG_INFO);
	Level_Editor_Log_Save("Ladataan uusi prototyyppi ");
	Level_Editor_Log_Save(tiedosto);	
	Level_Editor_Log_Save("\n");

	char tiedostopolku[_MAX_PATH];

	strcpy(tiedostopolku,polku);
	strcat(tiedostopolku,"/");

	
	if (seuraava_vapaa_proto < MAX_PROTOTYYPPEJA){
		if (protot[seuraava_vapaa_proto].Lataa(tiedostopolku, tiedosto) == 1){
			Level_Editor_Log_Write("loading sprite failed.", LOG_VIRHE);
			//strcpy(viesti,"could not load ");
			//strcat(viesti,tiedostopolku);
			//strcat(viesti,tiedosto);
			return 1;
		}
		
		strcpy(kartta->protot[seuraava_vapaa_proto],tiedosto);
		strcpy(viesti,"loaded sprite ");
		strcat(viesti,kartta->protot[seuraava_vapaa_proto]);
	}
	else{
		Level_Editor_Log_Write("too many sprites to load.", LOG_VIRHE);
		strcpy(viesti,"could not load any more sprites");
		return 2;
	}

	strcpy(tiedostopolku,polku);
	strcat(tiedostopolku,"/");

	seuraava_vapaa_proto++;
	
	return 0;
}

int Level_Editor_Prototyyppi_Lataa_Kaikki(){
	char tiedosto[_MAX_PATH];
	int spriteja = 0;
	int viimeinen_proto = 0;

	Level_Editor_Log_Write("loading all sprites.", LOG_INFO);

	for (int i=0;i < MAX_PROTOTYYPPEJA;i++){

		if (strcmp(kartta->protot[i],"") != 0){
			viimeinen_proto = i;
			strcpy(tiedosto,pk2_path);
			strcat(tiedosto,"/sprites");
			spriteja++;

			if (Level_Editor_Sprite_File_Load(tiedosto,kartta->protot[i])==1){
				Level_Editor_Log_Save("lataus epäonnistui.\n");
			}
			else
				Level_Editor_Log_Save("lataus onnistui.\n");
		}
		else
			seuraava_vapaa_proto++;

	}

	seuraava_vapaa_proto = viimeinen_proto+1;

	strcpy(tiedosto,"/sprites");

	if (spriteja == 0)
		strcpy(viesti,"map loaded without sprites");

	Level_Editor_Remove_Empty_Sprites();

	Level_Editor_Log_Save("Tyhjat prototyypit on poistettu");

	return 0;
}

void Level_Editor_Save_Undo(){
	//Level_Editor_Log_Write("making backup of map (undo)", LOG_INFO);
	undo->Kopioi(*kartta);
	varmistettu = true;
}

void Level_Editor_Map_Undo(){
	if (varmistettu){
		Level_Editor_Log_Write("used undo", LOG_INFO);
		strcpy(viesti,"used undo");
		kartta->Kopioi(*undo);
		varmistettu = false;
		Level_Editor_Aseta_Palikat(kartta->palikka_bmp);
		//Level_Editor_Aseta_Taustakuva(kartta->taustakuva);
	}
	else
		strcpy(viesti,"nothing to undo");
}

int Level_Editor_Map_Update(){
	UCHAR *buffer = NULL;
	DWORD tod_leveys;
	UCHAR vari_taka, vari_etu, vari, vari_sprite;

	PisteDraw_Piirto_Aloita(kuva_kartta, *&buffer, (DWORD &)tod_leveys);

	for (int x=0; x<PK2KARTTA_KARTTA_LEVEYS; x++){
		for (int y=0; y<PK2KARTTA_KARTTA_KORKEUS; y++){
			vari_taka = kartta->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS];
			vari_etu  = kartta->seinat [x+y*PK2KARTTA_KARTTA_LEVEYS];
			vari_sprite = kartta->spritet[x+y*PK2KARTTA_KARTTA_LEVEYS];
			
			vari = 0;
/*
			if (vari_taka != 255)
				vari = 12+128;//96;

			if (vari_taka >= 130 && vari_taka <= 139)
				vari = 12+32;			

			if (vari_etu != 255)
				vari = 24+128;//96;

			if (vari_etu != 255)
				vari = 24+96;			

			if (vari_etu >= 130 && vari_etu <= 139)
				vari = 20+32;*/
			
			if (vari_taka < 150)
				vari = karttavarit[vari_taka];

			if (vari_etu < 150 && vari_etu != 255)// && vari_etu != PALIKKA_ESTO_ALAS) TODO - ?
				vari = karttavarit[vari_etu];
			
			if (vari_sprite != 255)
				vari = 231;
			buffer[x+y*tod_leveys] = vari;
		}
	}

	PisteDraw_Piirto_Lopeta(kuva_kartta);

	Level_Editor_Calculate_Sprites();

	return 0;
}

int Level_Editor_Map_Defaults(){
	char tiedosto[_MAX_PATH];

	Level_Editor_Log_Write("loading map default settings.", LOG_INFO);

	if (strcmp(tile_path," ")!=0){
		//memset(tiedosto,'\0',sizeof(tiedosto));
		//strcpy(tiedosto,tile_path);
		//strcat(tiedosto,"tiles01.bmp");
		strcpy(tiedosto,"tiles01.bmp");
		Level_Editor_Aseta_Palikat(tiedosto);
		
		//PisteDraw_Lataa_Kuva(kartta->palikat_buffer,tiedosto,false);
	}

	if (strcmp(pk2_path," ")!=0){
		//memset(tiedosto,'\0',sizeof(tiedosto));
		strcpy(tiedosto,pk2_path);
		strcat(tiedosto,"/sprites");
		Level_Editor_Load_New_Sprite(tiedosto, "rooster.spr");
		
	}

	strcpy(kartta->musiikki,"song01.xm");
	strcpy(mapfile,"untitled.map");

	//strcpy(kartta->taustakuva,"default.bmp");
	//Level_Editor_Aseta_Taustakuva(kartta->taustakuva);


	return 0;
}

int Level_Editor_Kartta_Alusta(){
	Level_Editor_Log_Write("clearing map.", LOG_INFO);
	
	kartta->Tyhjenna();
	
	PisteDraw_Buffer_Tayta(kartta->taustakuva_buffer,37);

	Level_Editor_Remove_All_Sprites();
	
	Level_Editor_Map_Defaults();

	Level_Editor_Map_Update();
	
	Level_Editor_Log_Write("map cleared.", LOG_INFO);

	return 0;
}

//File loki.txt
int Level_Editor_Log_Save(char *viesti){
	if (kirjoita_loki){
		int virhe = 0;

		char *filename = "loki.txt";
			
		FILE *tiedosto;

		if ((tiedosto = fopen(filename, "a")) == NULL)
		{
			return(1);
		}

		char mjono[255];

		//memset(mjono,' ',sizeof(mjono));
		//mjono[60] = '\n';

		strcpy(mjono,viesti);

		fwrite(mjono,		sizeof(CHAR),	strlen(mjono),  tiedosto);

		fclose(tiedosto);
	}

	return(0);
}

int Level_Editor_Map_Save(){
	Level_Editor_Log_Write("trying to save map...", LOG_INFO);

	int virhe;

	char filename[_MAX_PATH]; 
	
	//strcpy(filename,kartta->nimi);
	strcpy(filename, mapfile);

	if (strstr(filename,".map") == NULL)
		strcat(filename,".map");

	if ((virhe = kartta->Tallenna(filename)) != 0){
		if (virhe == 1){
			Level_Editor_Log_Write("could not save map!", LOG_VIRHE);
			//strcpy(viesti,"could not save map!");
			return(1);
		}
		if (virhe == 2){
			strcpy(viesti,"error saving map!");
			Level_Editor_Log_Write("error occured while saving map!", LOG_VIRHE);
			return(1);
		}
	}
	
	Level_Editor_Log_Write("map saved succesfully!", LOG_INFO);
	strcpy(viesti,"map saved.");
	
	return(0);
}

int Level_Editor_Kartta_Lataa(char *filename){
	char d[_MAX_PATH] = "";
	getcwd(d, _MAX_PATH);
	printf("In %s\nStarts %s\n", d, filename);
	int virhe;

	char msg[200];
	strcpy(msg,"loading map: ");
	strcat(msg,filename);
	Level_Editor_Log_Write("----------------------", LOG_INFO);
	Level_Editor_Log_Write(msg, LOG_INFO);
	Level_Editor_Log_Write("----------------------", LOG_INFO);

	//Level_Editor_Log_Write("trying to load map.", LOG_INFO);

	//DWORD loki_muistia = PisteDraw_Videomuistia();
	//char muistia[255];
	//itoa(loki_muistia,muistia,10);
	
	//Level_Editor_Log_Save("Videomuistia: ");
	//Level_Editor_Log_Save(muistia);
	//Level_Editor_Log_Save("\n");

	Level_Editor_Log_Save("Aloitetaan kartan ");
	Level_Editor_Log_Save(filename);
	Level_Editor_Log_Save(" lataus.\n");

	Level_Editor_Save_Undo();

	//Level_Editor_Log_Save("Kartta varmistettu (undo).\n");

	Level_Editor_Remove_All_Sprites();

	//Level_Editor_Log_Save("Prototyypit tyhjennetty.\n");

	strcpy(mapfile, filename);

	if ((virhe = kartta->Lataa("",filename)) != 0){
		if (virhe == 1){
			Level_Editor_Log_Write("could not load map!", LOG_VIRHE);
			//Level_Editor_Log_Save("Kartan lataus epäonnistui.\n");
			strcpy(mapfile, " ");
			return(1);
		}
		
		if (virhe == 2){
			Level_Editor_Log_Write("this editor version cant read this map!", LOG_VIRHE);
			return(1);
		}
	}
	
	Level_Editor_Log_Write("map loaded", LOG_INFO);
	//strcpy(viesti,"map loaded: ");

	strcpy(mapfile, filename);

	Level_Editor_Map_Update();

	//Level_Editor_Log_Save("Kartta päivitetty.\n");
	
	
	kartta_ladattu = true;

	//Level_Editor_Log_Save("Kartta ladattu.\n");

	if (strcmp(kartta->versio,"1.2")==0 || strcmp(kartta->versio,"1.3")==0)
		Level_Editor_Prototyyppi_Lataa_Kaikki();
	else
		Level_Editor_Remove_All_Sprites();
/*
	char polku[_MAX_DIR];
	strcpy(polku,pk2_path);
	strcat(polku,"/");

	if (kartta->Lataa_PalikkaPaletti("",kartta->palikka_bmp)!=0)
	{
		if (kartta->Lataa_PalikkaPaletti(polku,kartta->palikka_bmp)!=0)
		{
			strcpy(viesti,"could not find tiles ");
			strcat(viesti,kartta->palikka_bmp);
		}
	}

	if (kartta->Lataa_Taustakuva("",kartta->taustakuva)!=0)
	{
		if (kartta->Lataa_Taustakuva(polku,kartta->taustakuva)!=0)
		{
			strcpy(viesti,"could not find background ");
			strcat(viesti,kartta->taustakuva);
		}
	}
*/
	Level_Editor_Log_Save("Sprite-prototyypit on ladattu.\n");

	menu_spritet_eka = 0;

	Level_Editor_Calculate_Sprites();

	Level_Editor_Laske_TileVarit();

	Level_Editor_Map_Update();

	Level_Editor_Log_Write("loading map completed.", LOG_INFO);

	return(0);
}

void Level_Editor_Laske_TileVarit(){
	
	UCHAR *buffer = NULL;
	DWORD leveys;
	int i;
	DWORD x,y;
	DWORD paavari;
	DWORD keskiarvoVari;
	int lapinakyvia;
	UCHAR vari;
	DWORD lkm;
	DWORD paavarit[9];
	
	Level_Editor_Log_Write("calculating tile colors.", LOG_INFO);

	PisteDraw_Piirto_Aloita(kartta->palikat_buffer,*&buffer,(DWORD &)leveys);

	for (int tile = 0; tile < 150; tile++){
		
		paavari = 0;
		keskiarvoVari = 0;
		lapinakyvia = 0;
		lkm = 0;
		
		for (i=0;i<8;i++)
			paavarit[i] = 0;

		for (x=0;x<32;x++){
			for (y=0;y<32;y++){
				
				vari = buffer[x + ((tile%10)*32) + (y+(tile/10)*32) * leveys];

				
				if (vari < 224){
					
					paavari = (vari/32);
					paavarit[paavari] = paavarit[paavari] + 1;
					keskiarvoVari = keskiarvoVari+(vari%32);
					lkm++;
				}
			}
		}

		paavari = 0;
		DWORD paavarilkm = 0;
		
		for (i=0;i<8;i++)
			if (paavarit[i] > paavarilkm){
				paavari = i;
				paavarilkm = paavarit[i];
			}

		if (lkm > 0)
			keskiarvoVari     = keskiarvoVari / lkm;
		else
			keskiarvoVari	  = 0;

		if (tile < 90)
			keskiarvoVari += 3;

		//TODO - ?
		//if (tile == PALIKKA_KYTKIN1 || tile == PALIKKA_KYTKIN2 || tile == PALIKKA_KYTKIN3 ||
		///	tile == PALIKKA_ALOITUS || tile == PALIKKA_LOPETUS)
		//	keskiarvoVari += 12;

		if (keskiarvoVari > 31)
			keskiarvoVari = 31;


		karttavarit[tile] = (UCHAR)(keskiarvoVari + paavari*32);
		//karttavarit[tile] = UCHAR(paavari*32);*/
	}


	PisteDraw_Piirto_Lopeta(kartta->palikat_buffer);

}

int Level_Editor_Menu_Alusta(int index, char *otsikko, int leveys, int korkeus, int x, int y, int pika){
	menut[index].korkeus = korkeus;
	menut[index].leveys = leveys;
	menut[index].x = x;
	menut[index].y = y;
	strcpy(menut[index].otsikko,otsikko);
	menut[index].piilota = true;
	menut[index].pika = pika;
	return 0;
}

int Level_Editor_Menut_Alusta(){
	Level_Editor_Menu_Alusta(MENU_HELP,		"help    (f1)", 256, 215, 192, 120, PI_F1);
	Level_Editor_Menu_Alusta(MENU_KARTTA,	"map     (f2)", PK2KARTTA_KARTTA_LEVEYS, PK2KARTTA_KARTTA_KORKEUS, 492, 130, PI_F2);
	Level_Editor_Menu_Alusta(MENU_PALIKAT,	"tiles   (f3)", 320, 480, 592, 145, PI_F3);
	Level_Editor_Menu_Alusta(MENU_SPRITET,	"sprites (f4)", 320, 480, 262, 45,  PI_F4);
	Level_Editor_Menu_Alusta(MENU_TIEDOSTOT,"files   (f5)", 320, 300, 192, 40,  PI_F5);
	Level_Editor_Menu_Alusta(MENU_SAVE,		"save    (f6)", 320, 50,  192, 355, PI_F6);
	Level_Editor_Menu_Alusta(MENU_INFO,		"map information (f7)", 320, 240,  150, 40, PI_F7);
	Level_Editor_Menu_Alusta(MENU_EXIT,		"exit editor?", 256, 35,  SCREEN_WIDTH/2-128, SCREEN_HEIGHT/2-15, PI_ESCAPE);
	Level_Editor_Menu_Alusta(MENU_TOOLS,	"tools   (f8)", 320, 200, 422, 210, PI_F8);
	Level_Editor_Menu_Alusta(MENU_LOKI,		"log     (f9)", 320, 500, 222, 110, PI_F9);

	menut[MENU_HELP].piilota = false;
	
	return 0;
}

int Level_Editor_Tiedostot_Alusta(){
	for (int i=0;i<MAX_DIR_FILES;i++){
		strcpy(tiedostot[i].name,"\0");
		tiedostot[i].type = 0;
	}

	return 0;
}

int Level_Editor_Tiedostot_Vertaa(char *a, char *b){
	int apituus = strlen(a);
	int bpituus = strlen(b);
	int looppi = apituus;
	int i;

	if (bpituus < apituus)
		looppi = bpituus;

	for (i=0 ; a[i]!='\0' ; i++)
		    	a[i]=tolower(a[i]);
	
	for (i=0 ; b[i]!='\0' ; i++)
		    	b[i]=tolower(b[i]);

	for (int i=0;i<looppi;i++){
		if (a[i] < b[i])
			return 2;
		
		if (a[i] > b[i])
			return 1;		
	}

	if (apituus > bpituus)
		return 1;

	if (apituus < bpituus)
		return 2;

	return 0;
}

int Level_Editor_Tiedostot_Aakkosta(){
	DWORD i,t, dirs;
	LEFILE temp;
	bool tehty;
	if (tiedostoja > 1LU){
		for (i=tiedostoja-1;i>=0;i--){
			tehty = true;

			for (t=0;t<i;t++){
				if (Level_Editor_Tiedostot_Vertaa(tiedostot[t].name,tiedostot[t+1].name) == 1){
					temp = tiedostot[t];
					tiedostot[t] = tiedostot[t+1];
					tiedostot[t+1] = temp;
					tehty = false;
				}
			}
			
			if (tehty)
				break;
		}
		
		for(i = 0; i < tiedostoja; i++){
			printf("%lu\n", tiedostoja);
			if (tiedostot[i].type == FILE_DIR){
				temp = tiedostot[dirs];
				tiedostot[dirs] = tiedostot[i];
				tiedostot[i] = temp;
				dirs++;
			}
		}
	}
	
	return 0;
}

int Level_Editor_Search_Files(){
	struct dirent **namelist;
	int index = 0;
	int i = 0;
	char *ext;

	int n = scandir("./", &namelist, 0, alphasort);
	if (n < 0) return -2;

	while(index < MAX_DIR_FILES && i < n){
		
		ext = strrchr(namelist[i]->d_name, '.');

		strcpy(tiedostot[index].name, namelist[i]->d_name);

		tiedostot[index].type = 0;


		if(namelist[i]->d_type == DT_DIR)
			tiedostot[index].type = FILE_DIR;
		else if(namelist[i]->d_type == DT_REG && ext != NULL){
			if(!strcmp(ext, ".map"))
				tiedostot[index].type = FILE_MAP;
			else if(!strcmp(ext, ".bmp"))
				tiedostot[index].type = FILE_BMP;
			else if(!strcmp(ext, ".pcx"))
				tiedostot[index].type = FILE_BMP;
			else if(!strcmp(ext, ".spr"))
				tiedostot[index].type = FILE_SPR;
			else if(!strcmp(ext, ".xm" ))
				tiedostot[index].type = FILE_MUS;
			else if(!strcmp(ext, ".mod"))
				tiedostot[index].type = FILE_MUS;
			else if(!strcmp(ext, ".s3m"))
				tiedostot[index].type = FILE_MUS;
			else if(!strcmp(ext, ".it" ))
				tiedostot[index].type = FILE_MUS;
		}
		if (tiedostot[index].type == 0) index --;
		index++;
		i++;
		
	}
	printf("Limpou\n");
	free(namelist);

	tiedostoja = index;

	return 0;
}

void Level_Editor_Aseta_Palikat(char *filename){
	Level_Editor_Log_Save("Ladataan palikkapaletti.\n");
	Level_Editor_Log_Write("loading tile palette.", LOG_INFO);

	if (kartta->Lataa_PalikkaPaletti("",filename) == 1)
		Error = true;

	Level_Editor_Laske_TileVarit();
	
}

void Level_Editor_Aseta_Taustakuva(char *filename){
	Level_Editor_Log_Save("Ladataan taustakuva.\n");
	Level_Editor_Log_Write("loading background image.", LOG_INFO);
	
	if (kartta->Lataa_Taustakuva("",filename) == 1)
		Error = true;
}

int Level_Editor_Init(){
	Level_Editor_Log_Start();
	Level_Editor_Log_Write("--------------------", LOG_INFO);
	Level_Editor_Log_Write("starting new session", LOG_INFO);
	Level_Editor_Log_Write("--------------------", LOG_INFO);
	Level_Editor_Log_Save("----------------------------------\n");
	Level_Editor_Log_Save("Uusi istunto.\n");
	Level_Editor_Log_Save("----------------------------------\n");

	for (int ci=0; ci<360; ci++) 
		cos_table[ci] = cos(3.1415*2* (ci%360)/180)*33;
    
	for (int si=0; si<360; si++) 
		sin_table[si] = sin(3.1415*2* (si%360)/180)*33;

	PK2Kartta_Cos_Sin(cos_table, sin_table);
	PK2Kartta_Aseta_Ruudun_Mitat(SCREEN_WIDTH,SCREEN_HEIGHT);

	getcwd(tyohakemisto, _MAX_PATH);
	
	strcpy(pk2_path,tyohakemisto);
	strcat(pk2_path,"/../res");

	strcpy(editor_path,tyohakemisto);

	Level_Editor_Start_PK2_Directory();
	
	Level_Editor_Tiedostot_Alusta();
	Level_Editor_Search_Files();
	Level_Editor_Tiedostot_Aakkosta();

	//MessageBox(0, "Welcome to Pekka Kana 2 Level Editor:)","PK2 Level Editor 0.91", MB_OK);

	if ((PisteInput_Alusta()) == PI_VIRHE)
		Error = true;

	if ((PisteDraw_Alusta(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_NAME)) == PD_VIRHE)
		Error = true;


	if ((kuva_editori = PisteDraw_Buffer_Uusi(SCREEN_WIDTH,SCREEN_HEIGHT, true, 255)) == PD_VIRHE)
		Error = true;

	if ((kuva_kartta  = PisteDraw_Buffer_Uusi(PK2KARTTA_KARTTA_LEVEYS,PK2KARTTA_KARTTA_KORKEUS, false, 255)) == PD_VIRHE)
		Error = true;	

	if (PisteDraw_Lataa_Kuva(kuva_editori,"../res/editor/pk2edit.bmp", true) == PD_VIRHE){
		Level_Editor_Log_Write("could not load pk2edit.bmp.", LOG_INFO);
		Error = true;
	}

	kartta  = new PK2Kartta();
	undo	= new PK2Kartta();

	if ((fontti1 = PisteDraw_Font_Uusi(kuva_editori,1,456,8,8,52)) == PD_VIRHE)
		Error = true;
	/*
	protot[PROTOTYYPPI_KANA].Kana("pk2spr01.bmp");
	protot[PROTOTYYPPI_MUNA].Muna("pk2spr01.bmp");
	protot[PROTOTYYPPI_PIKKUKANA].Pikkukana("pk2spr01.bmp");
	protot[PROTOTYYPPI_OMENA].Omena("pk2spr01.bmp");
*/
	Level_Editor_Kartta_Alusta();

	//Level_Editor_Map_Defaults();

	Level_Editor_Menut_Alusta();
	
	Level_Editor_Save_Undo();

	PisteDraw_Fade_Paletti_In(PD_FADE_NOPEA);

	//PisteWait_Start();

	Level_Editor_Log_Write("startup completed.", LOG_INFO);

	Level_Editor_Get_Settings();

	return 0;
}

/* PIIRTORUTIINIT ---------------------------------------------------------------*/

int Level_Editor_Draw_Nuolet(int x, int y, int menuId){
	int leveys = 13,
		korkeus = 27;

	PisteDraw_Buffer_Flip_Nopea(kuva_editori,PD_TAUSTABUFFER,x,y,324,1,337,28);

	if (PisteInput_Hiiri_Vasen() && key_delay == 0 && menuId==active_menu){
		if (mouse_x > x && mouse_x < x+leveys && mouse_y > y && mouse_y < y+korkeus/2){
			key_delay = 8;
			return 1;
		}
		
		if (mouse_x > x && mouse_x < x+leveys && mouse_y > y+korkeus/2 && mouse_y < y+korkeus){
			key_delay = 8;
			return 2;
		}
	}

	return 0;
}

int Level_Editor_Draw_Nuolet2(int x, int y, int menuId){
	int leveys = 13,
		korkeus = 13;

	PisteDraw_Buffer_Flip_Nopea(kuva_editori,PD_TAUSTABUFFER,x,y,   324, 1,337,15);
	PisteDraw_Buffer_Flip_Nopea(kuva_editori,PD_TAUSTABUFFER,x+14,y,324,15,337,28);

	if (PisteInput_Hiiri_Vasen() && key_delay == 0 && menuId==active_menu){
		if (mouse_x > x && mouse_x < x+13 && mouse_y > y && mouse_y < y+13){
			key_delay = 8;
			return 1;
		}
		
		if (mouse_x > x+14 && mouse_x < x+27 && mouse_y > y && mouse_y < y+13){
			key_delay = 8;
			return 2;
		}
	}

	return 0;
}

int Level_Editor_Draw_Nelio(int vasen, int yla, int oikea, int ala, UCHAR color){
	PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,vasen,yla,oikea,yla+1,color);
	PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,vasen,ala-1,oikea,ala,color);
	PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,vasen,yla,vasen+1,ala,color);
	PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,oikea-1,yla,oikea,ala,color);

	return 0;
}

void Level_Editor_Viiva_Hori(int x, int y, int pituus, UCHAR vari){
	PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,x,y,x+pituus,y+1,vari);
}

void Level_Editor_Viiva_Vert(int x, int y, int pituus, UCHAR vari){
	PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,x,y,x+1,y+pituus,vari);
}

bool Level_Editor_Nappi(char *otsikko, int vasen, int yla, int menuId){
	bool paalla = false;

	int vali = PisteDraw_Font_Kirjoita(fontti1,otsikko,PD_TAUSTABUFFER,vasen+2,yla+2);
	
	//UCHAR *buffer = NULL;
	//DWORD tod_leveys;
	UCHAR color = 18;

	int oikea = vasen+vali+2,
		ala   = yla+12;

	if (mouse_x < oikea && mouse_x > vasen && mouse_y > yla && mouse_y < ala && 
		(menuId == active_menu || menuId == -1)){
		paalla = true;
		color = 29;
	}

	Level_Editor_Draw_Nelio(vasen+1,yla+1,oikea+1,ala+1,0);
	Level_Editor_Draw_Nelio(vasen,yla,oikea,ala,color);

	if (paalla && PisteInput_Hiiri_Vasen() && key_delay == 0){
		key_delay = 20;
		return true;
	}
	
	return false;
}

void Level_Editor_List(TOOL_LIST &lista){
	if (lista.nakyva){
		int lista_y = 3;
		
		PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,lista.x, lista.y, lista.x + 58, lista.y + 75, 12);

		if (mouse_x > lista.x && mouse_x < lista.x+58 && mouse_y > lista.y && mouse_y < lista.y+75)
			edit_screen = EDIT_MENU;

		Level_Editor_Draw_Nelio(lista.x,lista.y,lista.x+58,lista.y+75,1);

		if (Level_Editor_Nappi("copy  ",lista.x+3,lista.y+lista_y,-1))	lista.valinta = 1; lista_y += 14;
		if (Level_Editor_Nappi("paste ",lista.x+3,lista.y+lista_y,-1))	lista.valinta = 2; lista_y += 14;
		if (Level_Editor_Nappi("cut   ",lista.x+3,lista.y+lista_y,-1))	lista.valinta = 3; lista_y += 14;
		if (Level_Editor_Nappi("fill  ",lista.x+3,lista.y+lista_y,-1))	lista.valinta = 4; lista_y += 14;
		if (Level_Editor_Nappi("cancel",lista.x+3,lista.y+lista_y,-1))   lista.valinta = 5; lista_y += 14;

		if (lista.valinta != 0) 
			lista.nakyva = false;
	}
	//else
	//	lista.valinta = 0;
}

bool Level_Editor_Link(char *otsikko, int vasen, int yla, int menuId){
	bool paalla = false;

	int vali = PisteDraw_Font_Kirjoita(fontti1,otsikko,PD_TAUSTABUFFER,vasen,yla);
	
	int oikea = vasen+200, //vali,
		ala   = yla+8;

	if (menuId != active_menu)
		return false;

	if (mouse_x < oikea && mouse_x > vasen && mouse_y > yla && mouse_y < ala)
		paalla = true;

	if (paalla){
		UCHAR *buffer = NULL;
		DWORD tod_leveys;
		
		PisteDraw_Piirto_Aloita(PD_TAUSTABUFFER, *&buffer, (DWORD &)tod_leveys);
	
		for (int x = vasen; x <= oikea; x++)
			for (int y = yla; y <= ala; y++)
				buffer[x+y*tod_leveys] = 41;
	
		PisteDraw_Piirto_Lopeta(PD_TAUSTABUFFER);

		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,otsikko,PD_TAUSTABUFFER,vasen,yla,90);
	}

	if (paalla && PisteInput_Hiiri_Vasen() && key_delay == 0){
		key_delay = 20;
		return true;
	}
	
	return false;
}

bool Level_Editor_Input(char *teksti, int vasen, int yla, int lkm, int id, int menuId){
	bool editoi = false;
	
	UCHAR color = 10;

	char merkki;

	int oikea = vasen+lkm*8,
		ala   = yla+10;	

	int pituus = strlen(teksti);

	int i;

	if (menuId == active_menu){
		if (mouse_x < oikea && mouse_x > vasen && mouse_y > yla && mouse_y < ala &&
			key_delay == 0 && PisteInput_Hiiri_Vasen()){
			editKursori = (mouse_x-vasen)/8;
			editKenttaId = id;
			memset(editTeksti,'\0',sizeof(editTeksti));
			strcpy(editTeksti,teksti); 
			key_delay = 20;

			if (editKursori > pituus)
				editKursori = pituus;	
		}

		if (id == editKenttaId){
			editoi = true;
			editing_text = true;
			color = 46;
		}


		if (/*editKursori < lkm &&*/ editoi){
			merkki = PisteInput_Lue_Nappaimisto();
			
			if (merkki != '\0' && key_delay == 0){
				if (merkki == '\n'){
					for (i=editKursori;i<lkm;i++)
						editTeksti[i] = '\0';

					editKenttaId = 0;
					key_delay = 20;
					editing_text = false;
				}
				else{
					editTeksti[editKursori] = merkki;

					pituus = strlen(editTeksti);

					if (editKursori < lkm-1)
						editKursori++;
					
					if (merkki == editMerkki)
						key_delay = 10;//5;
					else
						key_delay = 20;//15;
					
					editMerkki = merkki;
				}
			}
			
			if (key_delay == 0){
				if (PisteInput_Keydown(PI_BACK)){
					if (editKursori > 0){
						editKursori--;

						for (i=editKursori;i<lkm-1;i++)
							editTeksti[i] = editTeksti[i+1];
						
						editTeksti[lkm-1] = '\0';
						editTeksti[lkm] = '\0';
					}

					if (editMerkki == PI_BACK)
						key_delay = 5;
					else
						key_delay = 20;
					
					editMerkki = PI_BACK;

				}

				if (PisteInput_Keydown(PI_DELETE)){
					for (i=editKursori;i<lkm-1;i++)
						editTeksti[i] = editTeksti[i+1];
					
					editTeksti[lkm-1] = '\0';
					editTeksti[lkm] = '\0';

					if (editMerkki == 'D')
						key_delay = 5;
					else
						key_delay = 20;
					
					editMerkki = 'D';
				}

				if (PisteInput_Keydown(PI_LEFT)){
					if (editKursori > 0)
						editKursori--;
					
					key_delay = 5;
				}

				if (PisteInput_Keydown(PI_RIGHT)){
					if (editKursori < lkm-1 && editKursori < pituus)
						editKursori++;

					key_delay = 5;
				}		
			
			}
			
			editTeksti[lkm] = '\0';
		}
	}

	PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,vasen,yla,oikea,ala,color);
	
	if (editoi)
		PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,vasen+editKursori*8,yla,vasen+editKursori*8+8,ala,20);

	PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,teksti,PD_TAUSTABUFFER,vasen,yla+2,85);

	if (editoi)
		strcpy(teksti,editTeksti);

	return editoi;
}

//==================================================
//Menu Functions
//==================================================

int Level_Editor_Menu_Help(int i){
	int x = menut[i].x,
		y = menut[i].y,//+16,
		my = y+19;

	PisteDraw_Font_Kirjoita(fontti1,"f1 = help", PD_TAUSTABUFFER, x+3, my);			my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"f2 = map", PD_TAUSTABUFFER, x+3, my);			my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"f3 = tile palette", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"f4 = sprites", PD_TAUSTABUFFER, x+3, my);		my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"f5 = load map/files", PD_TAUSTABUFFER, x+3, my);		my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"f6 = save map", PD_TAUSTABUFFER, x+3, my);		my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"f7 = map information", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"f8 = clear map", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"s  = quick save ", PD_TAUSTABUFFER, x+3, my);	my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"u  = undo ", PD_TAUSTABUFFER, x+3, my);		my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"a  = animate moving tiles", PD_TAUSTABUFFER, x+3, my);	my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"shift = background / foreground", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"mouse left  = draw", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"mouse right = remove", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"mouse left  = paint (map menu)", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"mouse right = go to (map menu)", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"alt + mouse = select area", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"ctrl + c = copy", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"ctrl + b = paste simple", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"ctrl + v = paste pattern", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"ctrl + x = cut", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"arrows = scroll map", PD_TAUSTABUFFER, x+3, my);my+=9;
	PisteDraw_Font_Kirjoita(fontti1,"esc = exit", PD_TAUSTABUFFER, x+3, my);my+=9;

	return 0;
}

int Level_Editor_Menu_Map(int i){
	int x = menut[i].x,
		y = menut[i].y+15;

	PisteDraw_Buffer_Flip_Nopea(kuva_kartta,PD_TAUSTABUFFER, x, y, 0, 0, PK2KARTTA_KARTTA_LEVEYS, PK2KARTTA_KARTTA_KORKEUS);

	if (active_menu == i){
		if (menut[i].lista.nakyva)
			Level_Editor_List(menut[i].lista);
		
		RECT nelio = {0,0,0,0};
		nelio.left = mouse_x-(SCREEN_WIDTH/32)/2;
		nelio.top = mouse_y-(SCREEN_HEIGHT/32)/2;

		if (nelio.left < x)
			nelio.left = x;

		if (nelio.left+SCREEN_WIDTH/32 > x + menut[i].leveys)
			nelio.left = x + menut[i].leveys - SCREEN_WIDTH/32;		

		if (nelio.top < y)
			nelio.top = y;

		if (nelio.top+SCREEN_HEIGHT/32 > y + menut[i].korkeus)
			nelio.top = y + menut[i].korkeus - SCREEN_HEIGHT/32;

		nelio.right  = nelio.left + SCREEN_WIDTH/32;
		nelio.bottom = nelio.top + SCREEN_HEIGHT/32;

		Level_Editor_Draw_Nelio(nelio.left-1, nelio.top-1, nelio.right+1, nelio.bottom+1, 0);
		Level_Editor_Draw_Nelio(nelio.left, nelio.top, nelio.right, nelio.bottom, 31);
		Level_Editor_Draw_Nelio(nelio.left+1, nelio.top+1, nelio.right-1, nelio.bottom-1, 0);

		if (PisteInput_Hiiri_Vasen() && mouse_y > y && key_delay == 0){
			kartta_x = nelio.left - x;
			kartta_y = nelio.top - y;
		}
	}

	return 0;
}

int Level_Editor_Menu_Tiles(int i){
	int x = menut[i].x,
		y = menut[i].y+15,
		y_plus = menu_palikat_nayta_y * 32;

	char luku[10];

	PisteDraw_Buffer_Flip_Nopea(kartta->palikat_buffer,PD_TAUSTABUFFER, x, y, 0, 0+y_plus, 320, 480+y_plus);

	if (active_menu == i){
		RECT nelio = {0,0,0,0};
		
		nelio.left = (((mouse_x-x)/32)*32)+x;
		nelio.top =  (((mouse_y-y)/32)*32)+y;

		if (nelio.left < x)
			nelio.left = x;

		if (nelio.left+32 > x + menut[i].leveys)
			nelio.left = x + menut[i].leveys - 32;		

		if (nelio.top < y)
			nelio.top = y;

		if (nelio.top+32 > y + menut[i].korkeus)
			nelio.top = y + menut[i].korkeus - 32;

		nelio.right  = nelio.left + 32;
		nelio.bottom = nelio.top + 32;

		Level_Editor_Draw_Nelio(nelio.left+1, nelio.top+1, nelio.right+1, nelio.bottom+1, 0);
		Level_Editor_Draw_Nelio(nelio.left, nelio.top, nelio.right, nelio.bottom, 120);

		int px = (mouse_x-x)/32;
		int py = ((mouse_y-y)/32)*10;

		//itoa(px+py+menu_palikat_nayta_y*10+1,luku,10);
		sprintf(luku, "%i", px+py+menu_palikat_nayta_y*10+1);
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,luku,PD_TAUSTABUFFER,nelio.left+2,nelio.top+2,65);

		if (PisteInput_Hiiri_Vasen() && mouse_y > y)
		{
			edit_palikka = px + py + menu_palikat_nayta_y*10;
			proto_valittu = MAX_PROTOTYYPPEJA;
			key_delay = 15;
		}

		/*if (PisteInput_Hiiri_Oikea() && mouse_y > y && key_delay == 0)
		{
			menu_palikat_nayta_y += 2;
			if (menu_palikat_nayta_y > 2)
				menu_palikat_nayta_y = 0;
			key_delay = 15;
		}*/
	}

	return 0;
}

int Level_Editor_Menu_Sprites(int i){
	int x = menut[i].x,
		y = menut[i].y+16;
	int vali;
	
	PisteDraw_Aseta_Klipperi(PD_TAUSTABUFFER, menut[i].x, menut[i].y, menut[i].x+menut[i].leveys, menut[i].y+menut[i].korkeus+16);

	int piirto_y = 5, piirto_x = 15, rv_korkeus = 0;
	bool rivinvaihto = false;
	
	int eka_proto = menu_spritet_eka;
	int vika_proto = menu_spritet_eka+8;

	if (vika_proto > MAX_PROTOTYYPPEJA)
		vika_proto = MAX_PROTOTYYPPEJA;

	if (eka_proto < 0)
		eka_proto = 0;

	char luku[10];
	int j_luku = eka_proto;

	for (int proto=eka_proto; proto<=vika_proto; proto++){
		if (protot[proto].korkeus > 0){
			if (protot[proto].korkeus > rv_korkeus)
				rv_korkeus = protot[proto].korkeus;

			if (mouse_x > x+piirto_x-1 && mouse_x < x+piirto_x+protot[proto].kuva_frame_leveys+1/*leveys+1*/ &&
				mouse_y > y+piirto_y-1 && mouse_y < y+piirto_y+protot[proto].kuva_frame_korkeus+1/*korkeus+1*/ &&
				mouse_y < menut[i].y+menut[i].korkeus+16){
				Level_Editor_Draw_Nelio(x+piirto_x-1, y+piirto_y-1, 
										  x+piirto_x+protot[proto].kuva_frame_leveys+1,/*  .frame_leveys+1*/
										  y+piirto_y+protot[proto].kuva_frame_korkeus+1, /*korkeus+1*/ 31);

				if (key_delay == 0 && PisteInput_Hiiri_Vasen()){
					proto_valittu = proto;
					key_delay = 10;
				}
				
				if (key_delay == 0 && PisteInput_Keydown(PI_DELETE)){
					Level_Editor_Log_Write("deleting sprite.", LOG_INFO);
					Level_Editor_Remove_Sprite(proto);
					key_delay = 30;
				}
			}
			else{
				Level_Editor_Draw_Nelio(x+piirto_x-1, y+piirto_y-1, 
										  x+piirto_x+protot[proto].kuva_frame_leveys+1, 
										  y+piirto_y+protot[proto].kuva_frame_korkeus+1, 18);
			}
			
//			itoa(j_luku+1,luku,10);
//			PisteDraw_Font_Kirjoita(fontti1,luku,PD_TAUSTABUFFER,x+4,y+piirto_y);
			protot[proto].Piirra(x+piirto_x,y+piirto_y,0);
			PisteDraw_Font_Kirjoita(fontti1,protot[proto].nimi,PD_TAUSTABUFFER,x+120,y+piirto_y);
			PisteDraw_Font_Kirjoita(fontti1,kartta->protot[proto],PD_TAUSTABUFFER,x+120,y+piirto_y+10);
			
			if (protot[proto].tyyppi == TYYPPI_PELIHAHMO){
				if (proto == kartta->pelaaja_sprite)
					vali = PisteDraw_Font_Kirjoita(fontti1,"player: yes",PD_TAUSTABUFFER,x+120,y+piirto_y+20);
				else
					vali = PisteDraw_Font_Kirjoita(fontti1,"player: no",PD_TAUSTABUFFER,x+120,y+piirto_y+20);

				if (Level_Editor_Nappi("set player",x+125+vali,y+piirto_y+18,i))
					kartta->pelaaja_sprite = proto;
			}

			//itoa(j_luku+1,luku,10);
			sprintf(luku, "%i", j_luku);
			PisteDraw_Font_Kirjoita(fontti1,luku,PD_TAUSTABUFFER,x+4,y+piirto_y);

			//vali = PisteDraw_Font_Kirjoita(fontti1,"bonus: ",PD_TAUSTABUFFER,x+240,y+piirto_y);
			//PisteDraw_Font_Kirjoita(fontti1,protot[proto].bonus_sprite,PD_TAUSTABUFFER,x+240+vali,y+piirto_y);
			

			Level_Editor_Viiva_Hori(x,y+piirto_y-3,320,13);
			
			if (protot[proto].kuva_frame_korkeus > 30)
				piirto_y += protot[proto].kuva_frame_korkeus + 6;  //rv_korkeus + 4;
			else
				piirto_y += 36;

			piirto_x = 15;
			rv_korkeus = 0;
/*
			else
			{
				piirto_x += protot[proto].leveys + 4;
			}			
*/
			j_luku++;
		}
	}

	int nuoli = Level_Editor_Draw_Nuolet(x+menut[i].leveys-15,y+menut[i].korkeus-30,i);
	
	if (nuoli == 1 && menu_spritet_eka > 0){
		menu_spritet_eka--;
	}

	if (nuoli == 2 && menu_spritet_eka < MAX_PROTOTYYPPEJA-1){
		menu_spritet_eka++;
	}

	PisteDraw_Aseta_Klipperi(PD_TAUSTABUFFER,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

	return 0;
}

int Level_Editor_Menu_Files(int i){
	int x = menut[i].x,
		y = menut[i].y+16,
		linkki_y = 0;

	bool paivita = false;
	char otsikko[_MAX_DIR];

	if (menu_tiedostot_eka < 0)
		menu_tiedostot_eka = 0;

	if (menu_tiedostot_eka > MAX_DIR_FILES-2)
		menu_tiedostot_eka = MAX_DIR_FILES-2;

	int eka_tiedosto  = menu_tiedostot_eka;
	int vika_tiedosto = menu_tiedostot_eka+35;

	if (vika_tiedosto > MAX_DIR_FILES)
		vika_tiedosto = MAX_DIR_FILES;

	if (eka_tiedosto < 0)
		eka_tiedosto = 0;

	int ti = menu_tiedostot_eka;
	int tilask = 0;

	//for (int ti=eka_tiedosto;ti<vika_tiedosto;ti++) TODO
	while (linkki_y<33*9 && ti < MAX_DIR_FILES){
		if (tiedostot[ti].type != 0){
			if (tiedostot[ti].type == FILE_DIR && nayta_hakemistot){
				strcpy(otsikko,tiedostot[ti].name);
				PisteDraw_Buffer_Flip_Nopea(kuva_editori,PD_TAUSTABUFFER,x+5,y+linkki_y+3,338,1,346,8);
				//strcat(otsikko,"...");
				if (Level_Editor_Link(otsikko,x+5+10,y+linkki_y+3,i)){
					if (chdir(tiedostot[ti].name) == 0){
						getcwd(tyohakemisto, _MAX_PATH );
						paivita = true;
						strcpy(viesti,"changed directory");
						menu_tiedostot_eka = 0;
					}
					else{
						strcpy(viesti,"could not open directory");
						cout << tiedostot[ti].name << " Em -- " << tyohakemisto << endl; // TODO - Cant open files with case		
					}				
				}
				linkki_y += 9;
			}			
			
			switch (menu_tiedostot_nayta){
			case 1:	if (tiedostot[ti].type == FILE_MAP){
						if (Level_Editor_Link(tiedostot[ti].name,x+5,y+linkki_y+3,i))
							Level_Editor_Kartta_Lataa(tiedostot[ti].name);
						linkki_y += 9;
					}
					break;
			case 2:	if (tiedostot[ti].type == FILE_BMP){
						if (Level_Editor_Link(tiedostot[ti].name,x+5,y+linkki_y+3,i))
						{
							Level_Editor_Aseta_Palikat(tiedostot[ti].name);
							strcpy(viesti,"tile palette loaded from picture");
						}
						linkki_y += 9;
					}
					break;
			case 3:	if (tiedostot[ti].type == FILE_SPR){
						if (Level_Editor_Link(tiedostot[ti].name,x+5,y+linkki_y+3,i))
							if (Level_Editor_Load_New_Sprite(tyohakemisto,tiedostot[ti].name) != 0)//..Uusi
							{
								//strcpy(viesti,"could not load sprite ");
								//strcat(viesti,tiedostot[ti].name);
							}
						linkki_y += 9;
					}
					break;
			case 4:	if (tiedostot[ti].type == FILE_BMP){
						if (Level_Editor_Link(tiedostot[ti].name,x+5,y+linkki_y+3,i)){
							Level_Editor_Aseta_Taustakuva(tiedostot[ti].name);
							strcpy(viesti,"background loaded from picture");
						}
						linkki_y += 9;
					}
					break;
			case 5:	if (tiedostot[ti].type == FILE_MUS){
						if (Level_Editor_Link(tiedostot[ti].name,x+5,y+linkki_y+3,i)){
							//Level_Editor_Aseta_Taustakuva(tiedostot[ti].name);
							strcpy(kartta->musiikki, tiedostot[ti].name);
							strcpy(viesti,"loaded music.");
						}
						linkki_y += 9;
					}
					break;
			default:break;//linkki_y += 9; break;
			}
		}
		ti++;
	}

	Level_Editor_Viiva_Vert(x + menut[i].leveys - 102, y, 300, 41);

	if (Level_Editor_Nappi("save   ",x + menut[i].leveys - 96,y+2,i )){
		menut[MENU_SAVE].piilota = false;
		menu_tiedostot_eka = 0;
		
		if (strcmp(epd_path," ")!=0){
			if (chdir(epd_path) == 0){
				getcwd(tyohakemisto, _MAX_PATH );
				strcpy(viesti,"moved to last map folder");
				paivita = true;
			}
		}
	}
	
	Level_Editor_Viiva_Hori(x + menut[i].leveys - 102, y+20, 100, 41);
	
	if (Level_Editor_Nappi("maps   ",x + menut[i].leveys - 96,y+24,i)){
		menu_tiedostot_nayta = 1;
		menu_tiedostot_eka = 0;
		
		if (strcmp(epd_path," ")!=0){
			if (chdir(epd_path) == 0){
				getcwd(tyohakemisto, _MAX_PATH );
				strcpy(viesti,"moved to last map folder");
				paivita = true;
			}
		}
	}
	
	if (Level_Editor_Nappi("tiles  ",x + menut[i].leveys - 96,y+38,i)){
		menu_tiedostot_nayta = 2;
		menu_tiedostot_eka = 0;

		if (strcmp(tile_path," ")==0)
			strcpy(tile_path,epd_path);
		
		if (strcmp(tile_path," ")!=0){
			if (chdir(tile_path) == 0){
				getcwd(tyohakemisto, _MAX_PATH );
				strcpy(viesti,"moved to last map folder");
				paivita = true;
			}
		}
	}
	
	if (Level_Editor_Nappi("sprites",x + menut[i].leveys - 96,y+52,i)){
		menu_tiedostot_nayta = 3;
		menu_tiedostot_eka = 0;

		if (strcmp(pk2_path," ")!=0){
			char temp[_MAX_DIR];
			strcpy(temp, pk2_path);
			strcat(temp,"/sprites");
			if (chdir(temp) == 0){
				getcwd(tyohakemisto, _MAX_PATH );
				strcpy(viesti,"moved to sprites folder");
				paivita = true;
			}
		}
	}

	if (Level_Editor_Nappi("scenery",x + menut[i].leveys - 96,y+66,i)){
		menu_tiedostot_nayta = 4;
		menu_tiedostot_eka = 0;

		if (strcmp(bg_path," ")==0)
			strcpy(bg_path,epd_path);		

		if (strcmp(bg_path," ")!=0){
			if (chdir(bg_path) == 0){
				getcwd(tyohakemisto, _MAX_PATH );
				strcpy(viesti,"moved to last map folder");
				paivita = true;
			}
		}
	}

	if (Level_Editor_Nappi("music  ",x + menut[i].leveys - 96,y+80,i)){
		menu_tiedostot_nayta = 5;
		menu_tiedostot_eka = 0;

		if (strcmp(pk2_path," ")!=0){
			char temp[_MAX_DIR];
			strcpy(temp, pk2_path);
			strcat(temp,"/music");
			if (chdir(temp) == 0){
				getcwd(tyohakemisto, _MAX_PATH );
				strcpy(viesti,"moved to music folder");
				paivita = true;
			}
		}
	}

	if (nayta_hakemistot)
		if (Level_Editor_Nappi("hide dirs",x + menut[i].leveys - 96,y+110,i)) nayta_hakemistot = !nayta_hakemistot;
	else
		if (Level_Editor_Nappi("show dirs",x + menut[i].leveys - 96,y+110,i)) nayta_hakemistot = !nayta_hakemistot;

	if (paivita){
		Level_Editor_Tiedostot_Alusta();
		Level_Editor_Search_Files();
		Level_Editor_Tiedostot_Aakkosta();
	}

	int nuoli = Level_Editor_Draw_Nuolet(x+menut[i].leveys-15,y+menut[i].korkeus-30,i);
	
	if (nuoli == 1 && menu_tiedostot_eka > 0)
		menu_tiedostot_eka -= 15;

	if (nuoli == 2 && menu_tiedostot_eka < MAX_DIR_FILES-1)
		menu_tiedostot_eka += 15;

	return 0;
}

int Level_Editor_Menu_Save(int i){
	int x = menut[i].x,
		y = menut[i].y+16;

	PisteDraw_Font_Kirjoita(fontti1,"type the name of the level:", PD_TAUSTABUFFER, x+3, y+3);

	Level_Editor_Input(mapfile,x+3,y+15,12, KENTTA_FILE,i);

	if (Level_Editor_Nappi("save",x+3,y+30,i)){
		Level_Editor_Map_Save();	
		Level_Editor_Tiedostot_Alusta();
		Level_Editor_Search_Files();
		Level_Editor_Tiedostot_Aakkosta();
	}

	return 0;
}

int Level_Editor_Menu_Param(int i){
	int x = menut[i].x,
		y = menut[i].y+16,
		my = y+19,
		vali = 0;

	char luku[20];
	
	vali = PisteDraw_Font_Kirjoita(fontti1,"version", PD_TAUSTABUFFER, x+3, my);
	PisteDraw_Font_Kirjoita(fontti1,kartta->versio, PD_TAUSTABUFFER, x+3+vali+16, my);my+=12;

	vali = PisteDraw_Font_Kirjoita(fontti1,"name:", PD_TAUSTABUFFER, x+3, my);
	Level_Editor_Input(kartta->nimi,x+83,my,16,KENTTA_NIMI,i);my+=12;

	vali = PisteDraw_Font_Kirjoita(fontti1,"file name:", PD_TAUSTABUFFER, x+3, my);
	Level_Editor_Input(mapfile,x+83,my,12, KENTTA_FILE,i);my+=12;	

	vali = PisteDraw_Font_Kirjoita(fontti1,"creator:", PD_TAUSTABUFFER, x+3, my);
	Level_Editor_Input(kartta->tekija,x+83,my,29, KENTTA_TEKIJA,i);my+=12;

	vali = PisteDraw_Font_Kirjoita(fontti1,"tile picture:", PD_TAUSTABUFFER, x+3, my);
	PisteDraw_Font_Kirjoita(fontti1,kartta->palikka_bmp, PD_TAUSTABUFFER, x+3+vali+16, my);my+=12;

	vali = PisteDraw_Font_Kirjoita(fontti1,"background picture:", PD_TAUSTABUFFER, x+3, my);
	PisteDraw_Font_Kirjoita(fontti1,kartta->taustakuva, PD_TAUSTABUFFER, x+3+vali+16, my);my+=12;

	vali = PisteDraw_Font_Kirjoita(fontti1,"background music:", PD_TAUSTABUFFER, x+3, my);
	Level_Editor_Input(kartta->musiikki,x+3+vali+16,my,12, KENTTA_MUSIIKKI,i);my+=12;

	int nuoli;
	nuoli = Level_Editor_Draw_Nuolet2(x+108,my,i);
	if (nuoli == 2 && kartta->jakso > 0)
		kartta->jakso--;
	if (nuoli == 1)
		kartta->jakso++;

	if (kartta->jakso < 0)
		kartta->jakso = 0;

	//itoa(kartta->jakso,luku,10);
	sprintf(luku, "%i", kartta->jakso);
	vali = PisteDraw_Font_Kirjoita(fontti1,"level:", PD_TAUSTABUFFER, x+3, my);
	PisteDraw_Font_Kirjoita(fontti1,luku, PD_TAUSTABUFFER, x+3+vali+16, my);my+=12;
	my+=10;

	nuoli = Level_Editor_Draw_Nuolet2(x+108,my,i);
	if (nuoli == 2 && kartta->aika > 0)
		kartta->aika -= 10;
	if (nuoli == 1)
		kartta->aika += 10;
	
	if (kartta->aika < 0)
		kartta->aika = 0;

	int min = kartta->aika / 60,
		sek = kartta->aika % 60;

	vali = PisteDraw_Font_Kirjoita(fontti1,"time:", PD_TAUSTABUFFER, x+3, my);
	if (min<10)
		vali += PisteDraw_Font_Kirjoita(fontti1,"0", PD_TAUSTABUFFER, x+3+vali+16, my);
	//itoa(min,luku,10);	
	sprintf(luku, "%i", min);
	vali += PisteDraw_Font_Kirjoita(fontti1,luku, PD_TAUSTABUFFER, x+3+vali+16, my);

	if (sek<10)
		vali += PisteDraw_Font_Kirjoita(fontti1,":0", PD_TAUSTABUFFER, x+3+vali+16, my);	
	else
		vali += PisteDraw_Font_Kirjoita(fontti1,":", PD_TAUSTABUFFER, x+3+vali+16, my);
	
	//itoa(sek,luku,10);
	sprintf(luku, "%i", sek);
	vali += PisteDraw_Font_Kirjoita(fontti1,luku, PD_TAUSTABUFFER, x+3+vali+16, my);	
	
	my+=16;
	
	
	char tausta[100];

	switch(kartta->tausta){
	case TAUSTA_STAATTINEN				: strcpy(tausta,"no scrolling  ");break;
	case TAUSTA_PALLARX_HORI			: strcpy(tausta,"left and right");break;
	case TAUSTA_PALLARX_VERT			: strcpy(tausta,"up and down   ");break;
	case TAUSTA_PALLARX_VERT_JA_HORI	: strcpy(tausta,"free scrolling");break;
	default								: break;
	}
	
	vali = PisteDraw_Font_Kirjoita(fontti1,"background scrolling:", PD_TAUSTABUFFER, x+3, my);
	if (Level_Editor_Nappi(tausta,x+3+vali+16,my,i)){
		kartta->tausta++;
		kartta->tausta %= 4;
	}
	my+=16;

	switch(kartta->ilma){
	case ILMA_NORMAALI					: strcpy(tausta,"normal       ");break;
	case ILMA_SADE						: strcpy(tausta,"rain         ");break;
	case ILMA_METSA						: strcpy(tausta,"leaves       ");break;
	case ILMA_SADEMETSA					: strcpy(tausta,"rain + leaves");break;
	case ILMA_LUMISADE					: strcpy(tausta,"snow         ");break;
	default								: break;
	}

	vali = PisteDraw_Font_Kirjoita(fontti1,"special:", PD_TAUSTABUFFER, x+3, my);
	if (Level_Editor_Nappi(tausta,x+3+vali+16,my,i)){
		kartta->ilma++;
		kartta->ilma %= 5;
	}
	
	my+=19;	

	/* Kartan ikoni */
	vali = PisteDraw_Font_Kirjoita(fontti1,"map icon:", PD_TAUSTABUFFER, x+3, my);
	//itoa(kartta->ikoni+1,luku,10);
	sprintf(luku, "%i", kartta->ikoni+1);
	PisteDraw_Font_Kirjoita(fontti1, luku, PD_TAUSTABUFFER, x+3+vali+16, my);	
	
	nuoli = Level_Editor_Draw_Nuolet2(x+118,my,i);
	if (nuoli == 1)
		kartta->ikoni++;
	if (nuoli == 2)
		kartta->ikoni--;
	
	if (kartta->ikoni < 0)
		kartta->ikoni = 0;

	my+=16;

	/* Kartan x-kordinaatti */
	vali = PisteDraw_Font_Kirjoita(fontti1,"map x:", PD_TAUSTABUFFER, x+3, my);
	//itoa(kartta->x,luku,10);
	sprintf(luku, "%i", kartta->x);
	PisteDraw_Font_Kirjoita(fontti1, luku, PD_TAUSTABUFFER, x+vali+9, my);	
	nuoli = Level_Editor_Draw_Nuolet2(x+100,my,i);
	if (nuoli == 1)	kartta->x+= 15;
	if (nuoli == 2)	kartta->x-= 15;
	if (kartta->x < 0) kartta->x = 0;
	if (kartta->x > 620) kartta->x = 620;

	vali = PisteDraw_Font_Kirjoita(fontti1,"map y:", PD_TAUSTABUFFER, x+3+150, my);
	//itoa(kartta->y,luku,10);	
	sprintf(luku, "%i", kartta->y);
	PisteDraw_Font_Kirjoita(fontti1, luku, PD_TAUSTABUFFER, x+vali+9+150, my);	
	nuoli = Level_Editor_Draw_Nuolet2(x+100+150,my,i);
	if (nuoli == 1)	kartta->y+=15;
	if (nuoli == 2)	kartta->y-=15;
	if (kartta->y < 0) kartta->y = 0;
	if (kartta->y > 620) kartta->y = 620;

	my+=9;	
	
	return 0;
}

int Level_Editor_Menu_Tools(int i){
	int x = menut[i].x,
		y = menut[i].y+16;

	//PisteDraw_Font_Kirjoita(fontti1,"", PD_TAUSTABUFFER, x+3, y+3);

	//Level_Editor_Input(mapfile/*kartta->nimi*/,x+3,y+15,12, KENTTA_FILE);

	if (Level_Editor_Nappi("clear sprites",x+10,y+10,i)){
		if (i==active_menu){
			Level_Editor_Save_Undo();
			Level_Editor_Remove_All_Sprites();
		}
	}

	y += 25;

	if (Level_Editor_Nappi("clear unused sprites",x+10,y+10,i)){
		if (i==active_menu){
			Level_Editor_Save_Undo();
			Level_Editor_Delete_Unused_Sprites();
			menu_spritet_eka = 0;
			//Level_Editor_Remove_All_Sprites();
		}
	}

	y += 25;

	if (Level_Editor_Nappi("clear all",x+10,y+10,i)){
		if (i==active_menu){
			Level_Editor_Save_Undo();
			Level_Editor_Kartta_Alusta();
		}
	}

	y += 25;

	return 0;
}

int Level_Editor_Menu_Log(int i){
	int x = menut[i].x,
		y = menut[i].y+16;

	int yl = 0;

	for (int l=0;l<MAX_LOG_LABEL;l++){

		if (loki[l].type == LOG_VIRHE){
			PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER, x+3, y+3+l*9+3, x+7, y+7+l*9+3, 20+64);
		}
		
		if (loki[l].type == LOG_INFO){
			PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER, x+3, y+3+l*9+3, x+7, y+7+l*9+3, 20+96);
		}
		
		PisteDraw_Font_Kirjoita(fontti1,loki[l].teksti, PD_TAUSTABUFFER, x+3+8, y+3+l*9);
		
	}


	return 0;
}

int Level_Editor_Menu_Quit(int i){
	int x = menut[i].x,
		y = menut[i].y+16;

	PisteDraw_Font_Kirjoita(fontti1,"do you really want to quit?", PD_TAUSTABUFFER, x+3, y+3);

	if (Level_Editor_Nappi("quit",x+3,y+20,i)){
		PisteDraw_Fade_Paletti_Out(PD_FADE_NOPEA);
		exit_editor = true;
		key_delay = 15;	
	}

	if (Level_Editor_Nappi("cancel",x+45,y+20,i)){
		menut[i].piilota = true;
		key_delay = 15;		
	}

	if (Level_Editor_Nappi("save and quit",x+100,y+20,i)){
		if (Level_Editor_Map_Save()==0){
			PisteDraw_Fade_Paletti_Out(PD_FADE_NOPEA);
			exit_editor = true;
		}

		key_delay = 15;	
	}

	return 0;
}

int Level_Editor_Draw_Menu(int index, bool tayta, UCHAR vari){
	int leveys = menut[index].leveys;
	int korkeus = menut[index].korkeus;

	leveys = leveys - 14;

	//PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER, menut[index].x-1, menut[index].y-1, 
	//					   menut[index].x+menut[index].leveys+1, menut[index].y+16+korkeus, 0);

	Level_Editor_Draw_Nelio(menut[index].x-1, menut[index].y-1, 
							  menut[index].x+menut[index].leveys+1, menut[index].y+16+korkeus, 0);

	if (tayta){
		double kork = menut[index].korkeus/6.0;
		double ky = 0;
		for (int c=0;c<6;c++)	{
			PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER, menut[index].x, menut[index].y+15+int(ky), 
							   menut[index].x+menut[index].leveys,  menut[index].y+15+int(ky+kork), 6-c+vari);
			ky += kork;
		}
	}

	PisteDraw_Buffer_Flip_Nopea(kuva_editori,PD_TAUSTABUFFER, menut[index].x, menut[index].y, 
								1, 1, 5, 16);

	PisteDraw_Buffer_Flip_Nopea(kuva_editori,PD_TAUSTABUFFER, menut[index].x+4, menut[index].y, 
								6, 1, 6+leveys, 16);

	PisteDraw_Buffer_Flip_Nopea(kuva_editori,PD_TAUSTABUFFER, menut[index].x+2+leveys, menut[index].y, 
								311, 1, 323, 16);

	if (index == active_menu)
		PisteDraw_Font_Kirjoita(fontti1,menut[index].otsikko,PD_TAUSTABUFFER,menut[index].x+5,menut[index].y+3);
	else
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,menut[index].otsikko,PD_TAUSTABUFFER,menut[index].x+5,menut[index].y+3,50);

	switch(index){
		case MENU_HELP		: Level_Editor_Menu_Help(index);break;
		case MENU_KARTTA	: Level_Editor_Menu_Map(index);break;
		case MENU_PALIKAT	: Level_Editor_Menu_Tiles(index);break;
		case MENU_SPRITET	: Level_Editor_Menu_Sprites(index);break;
		case MENU_TIEDOSTOT	: Level_Editor_Menu_Files(index);break;
		case MENU_SAVE		: Level_Editor_Menu_Save(index);break;
		case MENU_INFO		: Level_Editor_Menu_Param(index);break;
		case MENU_TOOLS		: Level_Editor_Menu_Tools(index);break;
		case MENU_LOKI		: Level_Editor_Menu_Log(index);break;
		case MENU_EXIT		: Level_Editor_Menu_Quit(index);break;
		default				: break;
	}

	return 0;
}

int Level_Editor_Draw_Menus(){
	for (int i=0;i<N_OF_MENUS;i++)
		if (!menut[i].piilota && i != active_menu)
			Level_Editor_Draw_Menu(i, true, VARI_SININEN);

	if (active_menu != MENU_EI && active_menu < N_OF_MENUS)
		Level_Editor_Draw_Menu(active_menu, true, VARI_SININEN);	

	return 0;
}

//==================================================
//Draw functions
//==================================================

int Level_Editor_Draw_Sprites(){
	int proto;

	for (int x=0;x<SCREEN_WIDTH/32;x++){
		for (int y=0;y<SCREEN_HEIGHT/32;y++){
			proto = kartta->spritet[x+kartta_x+(y+kartta_y)*PK2KARTTA_KARTTA_LEVEYS];
			
			if (proto != 255 && protot[proto].tyyppi != TYYPPI_EI_MIKAAN)
				protot[proto].Piirra(x*32+16-protot[proto].leveys/2,y*32-protot[proto].korkeus+32,0);
		}
	}	
	return 0;
}

int Level_Editor_Draw_Map(){
	if (edit_kerros == EDIT_WALLS || edit_kerros == EDIT_BACKGROUND)
		kartta->Piirra_Taustat(kartta_x*32, kartta_y*32, animaatio_paalla);
	
	if (show_sprites)
		Level_Editor_Draw_Sprites(); //TODO
	
	if (edit_kerros == EDIT_WALLS || edit_kerros == EDIT_JUST_BACKGROUND)
		kartta->Piirra_Seinat(kartta_x*32, kartta_y*32, animaatio_paalla);

	return 0;
}

//Here you edit the map background and wall charts
int Level_Editor_Edit_Map(){
	int x = kartta_x + mouse_x / 32;
	int y = kartta_y + mouse_y / 32;

	if (y > 223)
		y = 223;

	if (edit_screen == EDIT_SPRITE || edit_screen == EDIT_MAP){
		if (!PisteInput_Hiiri_Vasen() && !PisteInput_Hiiri_Oikea())
			piirto_aloitettu = false;

		if ((PisteInput_Hiiri_Vasen() || PisteInput_Hiiri_Oikea()) && piirto_aloitettu == false){
			piirto_aloitettu = true;
			Level_Editor_Save_Undo();
		}
	}
	else
		piirto_aloitettu = false;


	if (edit_screen == EDIT_MAP){
		focustile_etu  = kartta->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS];
		focustile_taka = kartta->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS];

		if (PisteInput_Hiiri_Vasen() && key_delay == 0){
			
			if (edit_kerros == EDIT_WALLS || edit_kerros == EDIT_JUST_BACKGROUND)
				kartta->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS] = edit_palikka;

			if (edit_kerros == EDIT_BACKGROUND)
				kartta->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS] = edit_palikka;

			Level_Editor_Map_Update();
			strcpy(viesti,"map was updated.");
			key_delay = 5;
		}
		
		if (PisteInput_Hiiri_Oikea() && key_delay == 0){
			if (edit_kerros == EDIT_WALLS || edit_kerros == EDIT_JUST_BACKGROUND)
				kartta->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS] = 255;

			if (edit_kerros == EDIT_BACKGROUND)
				kartta->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS] = 255;

			//Level_Editor_Show_List(leikepoyta_lista, mouse_x, mouse_y);

			Level_Editor_Map_Update();
			strcpy(viesti,"map was updated.");
			key_delay = 5;
		}
		
	}

	if (edit_screen == EDIT_SPRITE){
		focussprite    = kartta->spritet[x+y*PK2KARTTA_KARTTA_LEVEYS];

		if (PisteInput_Hiiri_Vasen() && key_delay == 0){
			kartta->spritet[x+y*PK2KARTTA_KARTTA_LEVEYS] = proto_valittu;
			Level_Editor_Map_Update();
			strcpy(viesti,"map was updated.");
			key_delay = 5;
		}
		
		if (PisteInput_Hiiri_Oikea() && key_delay == 0){
			kartta->spritet[x+y*PK2KARTTA_KARTTA_LEVEYS] = 255;
			Level_Editor_Map_Update();
			strcpy(viesti,"map was updated.");
			key_delay = 5;
		}
	}

	return 0;
}

int Level_Editor_Draw_Info(){
	int vali = 0;
	char luku[40];

	if (edit_screen == EDIT_MAP){
		vali += PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"x: ",PD_TAUSTABUFFER,2+vali,2,65);
	
		//itoa(kartta_x+mouse_x/32,luku,10);
		sprintf(luku, "%i", kartta_x+mouse_x/32);
	
		vali += 8+PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,luku,PD_TAUSTABUFFER,2+vali,2,65);
	
		vali += PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"y: ",PD_TAUSTABUFFER,2+vali,2,65);
	
		//itoa(kartta_y+mouse_y/32,luku,10);
		sprintf(luku, "%i", kartta_y+mouse_y/32);
	
		vali += 10 + PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,luku,PD_TAUSTABUFFER,2+vali,2,65);

		
		if (focustile_etu != 255)
			//itoa(focustile_etu+1,luku,10);
			sprintf(luku, "%i", focustile_etu+1);
		else 
			strcpy(luku,"n/a");
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"foreground tile: ",PD_TAUSTABUFFER,2,14,65);
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,luku,PD_TAUSTABUFFER,150,14,65);

		if (focustile_taka != 255)
			//itoa(focustile_taka+1,luku,10);
			sprintf(luku, "%i", focustile_taka+1);
		else 
			strcpy(luku,"n/a");
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"background tile: ",PD_TAUSTABUFFER,2,24,65);
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,luku,PD_TAUSTABUFFER,150,24,65);

	}

	if (edit_screen == EDIT_SPRITE){
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"sprite: ",PD_TAUSTABUFFER,2,14,65);
		if (focussprite != 255)
			PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,protot[focussprite].nimi,PD_TAUSTABUFFER,90,14,65);		
		else
			PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"n/a",PD_TAUSTABUFFER,90,14,65);	
	}

	vali += PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,kartta->nimi,PD_TAUSTABUFFER,2+vali,2,65);

	PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,viesti,PD_TAUSTABUFFER,320,2,65);

	if (edit_kerros == EDIT_WALLS)
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"layer: both",PD_TAUSTABUFFER,320,12,65);
	if (edit_kerros == EDIT_BACKGROUND)
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"layer: only background",PD_TAUSTABUFFER,320,12,65);
	if (edit_kerros == EDIT_JUST_BACKGROUND)
		PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"layer: only foreground",PD_TAUSTABUFFER,320,12,65);
	
	PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"f1=help, f2=map, f3=tiles, f4=sprites, f5=files, f6=save map, f7=map info, f8=tools, f9=log, esc=exit",PD_TAUSTABUFFER,2,SCREEN_HEIGHT-10,75);

	//PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,tyohakemisto,PD_TAUSTABUFFER,2,SCREEN_HEIGHT-25,65);
	PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,PK2Kartta::pk2_hakemisto,PD_TAUSTABUFFER,2,SCREEN_HEIGHT-25,65);
	
	vali = PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"sprites: ",PD_TAUSTABUFFER,640,2,65);
	//itoa(spriteja,luku,10);
	sprintf(luku, "%i", spriteja);
	vali += PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,luku,PD_TAUSTABUFFER,640+vali,2,65);
	vali += PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,"/",PD_TAUSTABUFFER,640+vali,2,65);
	//itoa(MAX_SPRITES,luku,10);
	sprintf(luku, "%i", MAX_SPRITES);
	vali += PisteDraw_Font_Kirjoita_Lapinakyva(fontti1,luku,PD_TAUSTABUFFER,640+vali,2,65);

	return 0;
}

int Level_Editor_Draw_Cursor(){
	int x,y;
	x = (mouse_x/32)*32;
	y = (mouse_y/32)*32;

	if (x>SCREEN_WIDTH-32)
		x=SCREEN_WIDTH-32;
	if (y>SCREEN_HEIGHT-32)
		y=SCREEN_HEIGHT-32;
		
	int px = ((edit_palikka%10)*32);
	int py = ((edit_palikka/10)*32);
		
	if (edit_screen == EDIT_MAP){
		Level_Editor_Draw_Nelio(x+1, y+1, x+33, y+33, 0);
		PisteDraw_Buffer_Flip_Nopea(kartta->palikat_buffer,PD_TAUSTABUFFER, x, y, px, py, px+32, py+32);
		Level_Editor_Draw_Nelio(x, y, x+32, y+32, 57);
	}

	if (edit_screen == EDIT_SPRITE)
		protot[proto_valittu].Piirra(x+16-protot[proto_valittu].leveys/2,y-protot[proto_valittu].korkeus+32,0);
	
	PisteDraw_Buffer_Flip_Nopea(kuva_editori,PD_TAUSTABUFFER,mouse_x, mouse_y, 1, 33, 19, 51);

	return 0;
}

int Level_Editor_Draw(){
	int kytkin_anim;

	if (!animaatio_paalla){
		if (palikka_animaatio < 64)
			kytkin_anim = palikka_animaatio;
		else
			kytkin_anim = KYTKIN_ALOITUSARVO-64;

		PK2Kartta_Animoi(degree,palikka_animaatio/20,
			KYTKIN_ALOITUSARVO-kytkin_anim,KYTKIN_ALOITUSARVO-kytkin_anim,
			KYTKIN_ALOITUSARVO-kytkin_anim,true);
	}else
		PK2Kartta_Animoi(degree,palikka_animaatio/20,0,0,0,false);

	palikka_animaatio = 1 + palikka_animaatio % 99;//100

	PisteDraw_Buffer_Tayta(PD_TAUSTABUFFER,37);

	switch(kartta->tausta){
		case TAUSTA_STAATTINEN:
			PisteDraw_Buffer_Flip_Nopea(kartta->taustakuva_buffer,PD_TAUSTABUFFER,0,0);
			break;

		case TAUSTA_PALLARX_HORI:
			PisteDraw_Buffer_Flip_Nopea(kartta->taustakuva_buffer,PD_TAUSTABUFFER,0,0);
			PisteDraw_Buffer_Flip_Nopea(kartta->taustakuva_buffer,PD_TAUSTABUFFER,640,0);
			break;

		case TAUSTA_PALLARX_VERT:
			PisteDraw_Buffer_Flip_Nopea(kartta->taustakuva_buffer,PD_TAUSTABUFFER,0,0);
			PisteDraw_Buffer_Flip_Nopea(kartta->taustakuva_buffer,PD_TAUSTABUFFER,0,480);
			break;

		case TAUSTA_PALLARX_VERT_JA_HORI:
			PisteDraw_Buffer_Flip_Nopea(kartta->taustakuva_buffer,PD_TAUSTABUFFER,0,0);
			PisteDraw_Buffer_Flip_Nopea(kartta->taustakuva_buffer,PD_TAUSTABUFFER,640,0);
			PisteDraw_Buffer_Flip_Nopea(kartta->taustakuva_buffer,PD_TAUSTABUFFER,0,480);
			PisteDraw_Buffer_Flip_Nopea(kartta->taustakuva_buffer,PD_TAUSTABUFFER,640,480);
			break;

		default:
			break;
	}

	Level_Editor_Draw_Map();

	Level_Editor_Draw_Menus();

	Level_Editor_Draw_Info();

	Level_Editor_Leikepoyta_Piirra();

	if (leikepoyta_lista.nakyva)
		Level_Editor_List(leikepoyta_lista);

	Level_Editor_Draw_Cursor();

	if (active_menu == MENU_EI)
		Level_Editor_Edit_Map();
	
	//PisteWait_Wait(10);

	//PisteDraw_Paivita_Naytto();

	//PisteWait_Start();

	//Level_Editor_Log_Save("Piirtorutiinit on suoritettu.");

	return 0;
}

int Level_Editor_Menut(){
	
//	active_menu = MENU_EI;
	bool aktiivisia = false;

	//if (PisteInput_Lue_Nappaimisto())
	//	active_menu 

	for (int i=0; i < N_OF_MENUS; i++){
		if (PisteInput_Keydown(menut[i].pika) && key_delay == 0){
			if (menut[i].piilota)
				menut[i].piilota = false;
			else
				menut[i].piilota = true;

			key_delay = 15;
		}
		
		if (menut[i].x < 0)
			menut[i].x = 0;

		if (menut[i].x + menut[i].leveys > SCREEN_WIDTH)
			menut[i].x -= (menut[i].x + menut[i].leveys) - SCREEN_WIDTH;

		if (menut[i].y < 0)
			menut[i].y = 0;

		if (menut[i].y + menut[i].korkeus+16 > SCREEN_HEIGHT)
			menut[i].y -= (menut[i].y + 16 + menut[i].korkeus) - SCREEN_HEIGHT;		

		if (active_menu == i){
		
			if ((mouse_x > menut[i].x)   && (mouse_x < menut[i].x + menut[i].leveys) &&
				(mouse_y > menut[i].y)   && (mouse_y < menut[i].y + 16)              && 
				PisteInput_Hiiri_Vasen() && !moving_window){
				virt_x = mouse_x - menut[i].x;
				virt_y = mouse_y - menut[i].y;
				moving_window = true;
			}

			if (!PisteInput_Hiiri_Vasen())
				moving_window = false;

			if (moving_window){
				menut[i].x = mouse_x - virt_x;
				menut[i].y = mouse_y - virt_y;
				aktiivisia = true;
			}			

			if ((mouse_x > menut[i].x+menut[i].leveys-10) && (mouse_x < menut[i].x + menut[i].leveys) &&
				(mouse_y > menut[i].y) && (mouse_y < menut[i].y + 16) && PisteInput_Hiiri_Vasen()){
				menut[i].piilota = true;
				key_delay = 15;
			}
		
		}

		if ((mouse_x > menut[i].x) && (mouse_x < menut[i].x + menut[i].leveys) &&
			(mouse_y > menut[i].y) && (mouse_y < menut[i].y + menut[i].korkeus+16) && !menut[i].piilota
			&& (active_menu == MENU_EI || i == active_menu)){
			active_menu = i;
			aktiivisia = true;
		}

	}

	if (!aktiivisia){
		active_menu = MENU_EI;
		moving_window = false;
	}

	if (active_menu != MENU_EI)
		edit_screen = EDIT_MENU;
	else{
		if (proto_valittu == MAX_PROTOTYYPPEJA)
			edit_screen = EDIT_MAP;
		else
			edit_screen = EDIT_SPRITE;
	}

	return 0;
}

int Level_Editor_Cursor(){
	MOUSE pos = PisteInput_Hiiri();
	mouse_x = pos.x;
	mouse_y = pos.y;

	if (mouse_x > SCREEN_WIDTH)
		mouse_x = SCREEN_WIDTH;

	if (mouse_x < 0)
		mouse_x = 0;

	if (mouse_y > SCREEN_HEIGHT)
		mouse_y = SCREEN_HEIGHT;

	if (mouse_y < 0)
		mouse_y = 0;

	return 0;
}

int Level_Editor_Trimm(){

	if (mouse_x < 2)
		kartta_x -= 1;

	if (mouse_x > SCREEN_WIDTH-2)
		kartta_x += 1;	

	if (mouse_y < 2)
		kartta_y -= 1;

	if (mouse_y > SCREEN_HEIGHT-2)
		kartta_y += 1;	

	if (!editing_text){
		if (PisteInput_Keydown(PI_LEFT) && key_delay == 0){
			kartta_x--;
			key_delay = 5;
		}

		if (PisteInput_Keydown(PI_RIGHT) && key_delay == 0){
			kartta_x++;
			key_delay = 5;
		}
	}

	if (PisteInput_Keydown(PI_UP) && key_delay == 0){
		kartta_y--;
		key_delay = 5;
	}

	if (PisteInput_Keydown(PI_DOWN) && key_delay == 0){
		kartta_y++;
		key_delay = 5;
	}

	if (kartta_x < 0)
		kartta_x = 0;

	if (kartta_x > PK2KARTTA_KARTTA_LEVEYS  - SCREEN_WIDTH/32)
		kartta_x = PK2KARTTA_KARTTA_LEVEYS  - SCREEN_WIDTH/32;

	if (kartta_y < 0)
		kartta_y = 0;

	if (kartta_y > PK2KARTTA_KARTTA_KORKEUS - SCREEN_HEIGHT/32)
		kartta_y = PK2KARTTA_KARTTA_KORKEUS - SCREEN_HEIGHT/32;

	return 0;
}

//==================================================
//Main frames
//==================================================

int Level_Editor_Main(){
	if (!running)
		return(0);

	/* HAETAAN NÄPPÄIMISTÖN, HIIREN JA PELIOHJAINTEN TÄMÄNHETKISET TILAT */

	// Näppäimistö 
	if (!PisteInput_Hae_Nappaimet())		//Haetaan näppäinten tilat
		Error = true;
	
	// Hiirulainen
	if (!PisteInput_Hae_Hiiri())			//Haetaan hiiren tila
		Error = true;	

	//editing_text = false;

	Level_Editor_Cursor();

	Level_Editor_Trimm();

	editing_text = false;

	Level_Editor_Draw();

	Level_Editor_Menut();

	if (key_delay > 0)
		key_delay--;

	degree++;
	degree %= 360;

	if (PisteInput_Keydown(PI_RALT)){
		if (!aseta_leikepoyta_alue){
			leikepoyta_alue.left = (mouse_x/32)+kartta_x;
			leikepoyta_alue.top  = (mouse_y/32)+kartta_y;
			aseta_leikepoyta_alue = true;
		}

		leikepoyta_alue.right  = (mouse_x/32)+kartta_x;
		leikepoyta_alue.bottom = (mouse_y/32)+kartta_y;

		long vaihto;

		if (leikepoyta_alue.right < leikepoyta_alue.left){
			vaihto = leikepoyta_alue.right;
			leikepoyta_alue.right = leikepoyta_alue.left;
			leikepoyta_alue.left = vaihto;
		}

		if (leikepoyta_alue.bottom < leikepoyta_alue.top){
			vaihto = leikepoyta_alue.top;
			leikepoyta_alue.top = leikepoyta_alue.bottom;
			leikepoyta_alue.bottom = vaihto;
		}

	}
	else
		if (aseta_leikepoyta_alue)
			aseta_leikepoyta_alue = false;


	if (key_delay == 0 && !editing_text){
		if (PisteInput_Keydown(PI_LCONTROL)){
			//Level_Editor_Save_Undo();
			if (PisteInput_Keydown(PI_C)){	
				Level_Editor_Leikepoyta_Kopioi(leikepoyta_alue);
				key_delay = 20;
			}
			if (PisteInput_Keydown(PI_V)){		
				Level_Editor_Leikepoyta_Liita(leikepoyta_alue);
				key_delay = 20;
			}
			if (PisteInput_Keydown(PI_X)){		
				Level_Editor_Leikepoyta_Leikkaa(leikepoyta_alue);
				key_delay = 20;
			}
			if (PisteInput_Keydown(PI_B)){		
				RECT temp = {mouse_x/32+kartta_x,mouse_y/32+kartta_y,mouse_x/32+kartta_x,mouse_y/32+kartta_y};
				Level_Editor_Leikepoyta_Liita_Koko(temp);
				key_delay = 20;
			}
		}

		if (PisteInput_Keydown(PI_S)){
			Level_Editor_Map_Save();
			key_delay = 20;
		}

		if (PisteInput_Keydown(PI_A)){
			animaatio_paalla = !animaatio_paalla;
			key_delay = 20;
		}

		if (PisteInput_Keydown(PI_U)) {
			Level_Editor_Map_Undo(); //TODO - U - undo
			Level_Editor_Map_Update();
			key_delay = 20;
		}

		if (PisteInput_Keydown(PI_RSHIFT)){
			switch(edit_kerros){
			case EDIT_WALLS:		edit_kerros = EDIT_JUST_BACKGROUND;break;
			case EDIT_JUST_BACKGROUND:	edit_kerros = EDIT_BACKGROUND;break;
			case EDIT_BACKGROUND:		edit_kerros = EDIT_WALLS;break;
			default:						break;
			}

			key_delay = 20;
		}

	}

	if (exit_editor && PisteDraw_Fade_Paletti_Valmis()){
		//SendMessage(0, WM_CLOSE,0,0);
		running = false;
	}

	return 0;
}

int Level_Editor_Quit(){

	delete kartta;
	delete undo;

	if (Error)
		strcpy(virheviesti,"ERROR!");

	Level_Editor_Save_Settings();

	return 0;
}

//TODO: Save "backup.map"
int main(int argc, char *argv[]){
	
	Piste_Init();

	Level_Editor_Init();
	
	//PisteDraw_Alusta_Debugger();

	Piste_Loop(running,*Level_Editor_Main);
	
	Level_Editor_Quit();

	Piste_Quit();

	return 0;
}

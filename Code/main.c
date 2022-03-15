#include <stdio.h>
#include <gb\gb.h>
#include <stdlib.h>
extern const unsigned char shortile[];        //спрайты персонажей и движущихся частей 
                                              
extern const unsigned char FInal[];           //спрайты фона
extern const unsigned char levelMap[];        //карта уровня
extern const unsigned char keyboard[];        //массив, содержащий карту клавиатуры
extern const unsigned char rec[];             //массив, содержащий карту таблицы рекордов
extern const unsigned char starstar_data[];   //массив, содержащий спрайты для начального экрана
extern const unsigned char starstar_map[];    //массив, сожержащий карту начального экрана
extern const unsigned char menuMAP[];         //массив, содержащий карту меню
extern const unsigned char dead[];            //массив, содержащий карту экрана, появляющегося после смерти
const char blankmap[3]= {0x00, 0x04, 0x0A};
struct object
{
    UBYTE spriteID[4];
    UINT8 x, y;
    UINT8 limits[4];
    UBYTE status;
};
struct coursor
{
    UINT8 x, y, col, row;
};
struct coursor dot;     //курсор

struct object star;     //игрок

struct object sp1;      //движущиеся шипы
struct object sp2;
struct object sp3;

struct object hat;      //шляпа

struct object j1;       //враги
struct object j2;
struct object j3;
struct object j4;

extern UINT8 records[91];                //массивы с информацией о рекордах 
extern UINT16 score[4];
extern UINT8 grscore[28];

INT8 currentSpeedX, currentSpeedY;
BYTE jumping;
BYTE flipFlag;
UBYTE coinsAmount[5]= {0, 0, 0, 0, 0};   //массив, содержащий в себе информацию о собранных монетах
BYTE shooting;

const UINT8 mincursorx = 12;             //границы перемещения курсора клавиатуры
const UINT8 mincursory = 80;
const UINT8 maxcursorx = 156;
const UINT8 maxcursory = 128;
UINT8 playername[13]=                    //массив, содержащий имя игрока, изначально заполнен пробелами
{81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81};

UINT8 namecharacterindex=0;              //число символов в имени
UBYTE keydown, playerhasname;

UINT8 resREC[91];                        //резервные массивы, в которые копируется таблица рекордов перед редактированием
UINT8 resSCORE[28];

UBYTE lifes;                             //число жизней
UINT16 points;                           //число очков
UBYTE exitAM;
UINT8 coins[10]=                         //массив, содержащий координаты монет
{11, 3, 19, 2, 19, 7, 0, 6, 2, 11};

/*
После смерти уровень полностью перезапускается,
 все монеты тоже возвращаются на свои места.
 Данная процедура скрывает от игрока уже собранные ранее монеты.
*/
void coinsrecover(UBYTE money[5]){        
    if (money[0]==1){                                      //Проверяется, собрал ли игрок монету                 
        set_bkg_tiles(coins[0], coins[1], 1, 1, blankmap); //Место, где находится эта монета, закрашивается
    }
    if (money[1]==1){
        set_bkg_tiles(coins[2], coins[3], 1, 1, blankmap);
    }
    if (money[2]==1){
        set_bkg_tiles(coins[4], coins[5], 1, 1, blankmap);
    }
    if (money[3]==1){
        set_bkg_tiles(coins[6], coins[7], 1, 1, blankmap);
    }
    if (money[4]==1){
        set_bkg_tiles(coins[8],coins[9], 1, 1, blankmap);
    }

}
/*
Задержка
*/
void perfDelay(UINT8 numloops){
    UINT8 i;
    for (i=1;i!= numloops; i++){
        wait_vbl_done();
    }
}
/*
Игрок отзеркаливается при изменении направления движения.
Данная процедура возвращает спрайты игрока в изначальное положение.
Когда после смерти игрок появляется в начале уровня, могут возникнуть артефакты, если
не вернуть все на свои места 
*/
void starrecover(){
    UINT8 i;
    for (i=1; i!=7; i++){
        set_sprite_prop(i, get_sprite_prop(0));
    }
}
//затемнение, экран абсолютно черный после этой процедуры
void fadeout(){
    UINT8 i;
	for(i=0;i!=4;i++){
		switch(i){
			case 0:
				BGP_REG = 0xE4;
				break;
			case 1:
				BGP_REG = 0xF9;
				break;
			case 2:
				BGP_REG = 0xFE;
				break;
			case 3:
				BGP_REG = 0xFF;	
				break;						
		}
		perfDelay(5);
	}
}
//возвращает экран к обычной 4хцветной палитре после затемнения
void fadein(){
    UINT8 i;
	for(i=0;i!=3;i++){
		switch(i){
			case 0:
				BGP_REG = 0xFE;
				break;
			case 1:
				BGP_REG = 0xF9;
				break;
			case 2:
				BGP_REG = 0xE4;
				break;					
		}
		perfDelay(5);
	}
}

//передвигает 4 спрайта как единое целое
void moveGameSprites(struct object* newobject, UINT8 x, UINT8 y ){
    move_sprite(newobject->spriteID[0], x, y);
    move_sprite(newobject->spriteID[1], x+ 8, y);
    move_sprite(newobject->spriteID[2], x, y+ 8);
    move_sprite(newobject->spriteID[3], x+ 8, y+ 8);
}

//обновляет число очков в уровне
void scoreUPDATE(UINT16 result){
    BYTE i, n;
    UINT8 arr[4];
    for (i=3; i!=-1; i--){
        n=result% 10;
        result/=10;
        arr[i]= n+ 82;
    }
    set_bkg_tiles(6, 17, 4, 1, arr);
}
/*
Привязывает спрайты медузы к цифрам, которые получает в параметрах
Добавляет в структуру медузы номера спрайтов (необходимо для передвижения медузы)
Добавляет в структуру медузы границы, до которых может двигаться медуза
*/
void setJelly(struct object* jelly1, UINT8 n1, UINT8 n2, UINT8 n3, UINT8 n4, UBYTE stat, UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2){
    set_sprite_tile(n1, 9);
    jelly1->spriteID[0]= n1;
    set_sprite_tile(n2, 10);
    jelly1->spriteID[1]= n2;
    set_sprite_tile(n3, 11);
    jelly1->spriteID[2]= n3;
    set_sprite_tile(n4, 12);
    jelly1->spriteID[3]= n4;
    jelly1->status= stat;
    jelly1->limits[0]= x1;
    jelly1->limits[2]= x2;
    jelly1->limits[1]= y1;
    jelly1->limits[3]= y2;
    if (stat==0){
        jelly1->x= x1;
        jelly1->y= y1;
    }else{
        jelly1->x= x2;
        jelly1->y= y2;
    }
    moveGameSprites(jelly1, x1, y1);
}
/*
Привязывает определенные цифры к спрайтам игрока, это необходимо для передвижения игрока
Вносит начальные координаты игрока в структуру игрока
Перемещает игрока в начало уровня
*/
void setStar(){
    star.x= 8;
    star.y= 24;
    set_sprite_tile(0, 0);
    set_sprite_tile(1, 1);
    star.spriteID[0]= 1;
    set_sprite_tile(2, 2);
    star.spriteID[1]= 2;
    set_sprite_tile(3, 5);
    star.spriteID[2]= 3;
    set_sprite_tile(4, 6);
    star.spriteID[3]= 4;
    set_sprite_tile(5, 3);                 
    set_sprite_tile(6, 4);
    set_sprite_tile(7, 7);                 
    moveGameSprites(&star, star.x, star.y);
}
/*
Привязывает спрайт шипа к цифре, которую получает в параметрах
Добавляет в структуры шипа номер спрайта (необходимо для передвижения шипа)
Добавляет в структуру шипа границы, до которых он может двигаться
*/
void setSpikes(struct object* spike1, UBYTE number, UBYTE stat, UINT8 x1, UINT8 y1, UINT8 x2, UINT8 y2 ){
    set_sprite_tile(number, 8);
    spike1->limits[0]= x1;
    spike1->limits[2]= x2;
    spike1->spriteID[0]= number;
    spike1->limits[1]= y1;
    spike1->limits[3]= y2;
    if (stat==0){
        spike1->x= x1;
        spike1->y= y1;
    }else{
        spike1->x= x2;
        spike1->y= y2;
    }
    spike1->status= stat;
    move_sprite(number, spike1->x, spike1->y);
}
/*
Процедура смерти.
Затемняет экран, после возвращает 4хцветную палитру, затем показывает экран смерти, 
после снова затемняет экран, загружает уровень, обновляет число очков и жизней, скрывает от
игрока собранные монеты, приводит спрайты игрока в изначальное положение, возвращает игрока в начало уровня
Затем возвращает 4хцветную палитру и игрок видит перезапущенный уровень
*/
void death(){
    UINT8 arr[1];
    fadeout();
    HIDE_SPRITES;
    lifes--;
    set_bkg_tiles(0, 0, 20, 18, dead);
    arr[0]= lifes +82;
    set_bkg_tiles(10, 9, 1, 1, arr);
    fadein();
    perfDelay(500);
    fadeout();
    if (lifes== 0){
        return;
    }
    set_bkg_tiles(0, 0, 20, 18, levelMap);
    set_bkg_tiles(19, 17, 1, 1, arr);
    scoreUPDATE(points);
    coinsrecover(coinsAmount);
    shooting= 0;
    jumping= 0;
    flipFlag= 1;
    starrecover();
    setStar();
    SHOW_SPRITES;
    fadein();
}
/*
Вызов данной функции происходит при любом перемещении игрока или его шляпы при атаке.
Одна из самых важных процедур.
В параметрах процедуры структура обьекта и новые координаты
По координатам вычисляется номер тайла в массиве тайлов заднего фона
Если значение этого тайла совпадает со значением тайла платформы, то возвращает 0
Если значение тайла совпадает со значением тайла шипа, то возвращает 0 и запускает процедуру смерти
Если значение тайла совпадает со значением тайла двери, уровень завершается
Если объект, для которого ведется проверка, не является шляпой (в массиве номеров тайлов заполнены все 4 ячейки),
в данной функции происходит сбор монет
*/
BYTE couldMove(struct object* player, UINT8 xCoordinate, UINT8 yCoordinate){
    UINT16 tileIDx, tileIDy, tileINDEX;
    tileIDx= (xCoordinate- 8)/ 8;                                                   //вычисление номера тайла
    tileIDy= (yCoordinate- 16)/ 8;
    tileINDEX= (20*tileIDy)+ tileIDx;
    if ((tileINDEX== 279) && (player->spriteID[1]!=0)){                             //дверь
        exitAM= 1;
    }
    if (levelMap[tileINDEX]==blankmap[1]){                                          //платформа
        return 0;
    }
    if (levelMap[tileINDEX]==blankmap[2]){                                          //шип статический
        death();
        return 0;
    }
    if ((tileINDEX== 71) && (coinsAmount[0]!= 1) && (player->spriteID[1]!=0)){      //первая монета
        coinsAmount[0]= 1;                                                          //монета собрана, первый элемент массива собранных монет равен 1
        set_bkg_tiles(tileIDx, tileIDy, 1, 1, blankmap);                            //тайл, где была монета, закрашивается пустым тайлом
        points+=500;                                                                //увеличиваются очки
        scoreUPDATE(points);                                                        //на экране уровня обновляется чило очков
        return 1;
    }else if ((tileINDEX== 59) && (coinsAmount[1]!= 1) && (player->spriteID[1]!=0)) //вторая монета
    {
        coinsAmount[1]= 1;
        set_bkg_tiles(tileIDx, tileIDy, 1, 1, blankmap);
        points+=500;
        scoreUPDATE(points);
        return 1;
    }else if ((tileINDEX== 159) && (coinsAmount[2]!= 1) && (player->spriteID[1]!=0)) //третья монета
    {
        coinsAmount[2]= 1;
        set_bkg_tiles(tileIDx, tileIDy, 1, 1, blankmap);
        points+=500;
        scoreUPDATE(points);
        return 1;
    }else if ((tileINDEX== 120) && (coinsAmount[3]!= 1) && (player->spriteID[1]!=0)) //четвертая монета
    {
        coinsAmount[3]= 1;
        set_bkg_tiles(tileIDx, tileIDy, 1, 1, blankmap);
        points+=500;
        scoreUPDATE(points);
        return 1;
    }else if ((tileINDEX== 222) && (coinsAmount[4]!= 1) && (player->spriteID[1]!=0)) //пятая монета
    {
        coinsAmount[4]= 1;
        set_bkg_tiles(tileIDx, tileIDy, 1, 1, blankmap);
        points+=500;
        scoreUPDATE(points);
        return 1;
    }
    return 1;
}
//данная процедура отражает игрока по вертикали при смене направления движения
//подробнее в презентации
void reverse(struct object* player){
    UBYTE n, i;
    for (i=0; i!=4; i+=2){
        n= player->spriteID[i];
        player->spriteID[i]= player->spriteID[i+1];
        player->spriteID[i+1]= n;
    }
    
    if (flipFlag== 1){
        set_sprite_prop(player->spriteID[0], S_FLIPX);
        set_sprite_prop(player->spriteID[1], S_FLIPX);
        set_sprite_prop(player->spriteID[2], S_FLIPX);
        set_sprite_prop(player->spriteID[3], S_FLIPX);
    }else{
        set_sprite_prop(player->spriteID[0], get_sprite_prop(0));
        set_sprite_prop(player->spriteID[1], get_sprite_prop(0));
        set_sprite_prop(player->spriteID[2], get_sprite_prop(0));
        set_sprite_prop(player->spriteID[3], get_sprite_prop(0));
    }
    moveGameSprites(player, player->x, player->y);
}
/*
Функция для детекции столкновений мета спрайта(комбинации из 4 спрайтов) с одним спрайтом
Первый указатель в параметрах обязательно должен быть на структуру одного спрайта
Второй - на структуру мета спрайта
Если расстояние между центрами объектов <12 пикселей, то возвращает 1, иначе 0
Отдельная функция понадобилась из-за того что координаты мета спрайта - всегда его центр, а
координаты единственного спрайта - его правый нижний угол. Так же из-за того, что ширина одного спрайта - 8 пикселей, а ширина метаспрайта 16
*/
BYTE onespritecollision(struct object* player1, struct object* player2){
    return((abs(player1->x-4 - player2->x)< 12) && (abs(player1->y-4 - player2->y)< 12));
}
/*
Функция для детекции столкновений между мета спрайтами
Если расстояние между центрами двух метаспрайтов <16, возвращает 1, иначе 0
*/
BYTE collision(struct object* player1, struct object* player2){
    return((abs(player1->x - player2->x)< 16) && (abs(player1->y - player2->y)< 16));
}
/*
Вызов данной процедуры происходит когда после атаки шляпа долетает до врага
Очки увеличиваются, а враг перемещается за экран, в недоступную для игрока зону
*/
void killin(struct object* enemy){
    enemy->x= 160+ 16;
    points+= 150;
    scoreUPDATE(points);
    moveGameSprites(enemy, enemy->x, enemy->y);
}
/*
"Физика" всего уровня в одной процедуре
Сначала в данной процедуре идет проверка делимости х-координаты игрока на 8
Если остаток равен 0, то под игроков ровно 2 тайла заднего фона
    Далее эти 2 тайла проверяются на "проходимость", если они не являются платформами, игрок начинает падать
Если же остаток не равен нулю, то под игроком 3 тайла заднего фона,
     происходит все то же самое, что в случае с двумя тайлами, но для трех тайлов
*/
void falling(struct object* player){
    if (player->x%8!= 0){
        if (couldMove(player, player->x- 4, player->y + 16) && couldMove(player,player->x+ 4, player->y+ 16) && couldMove(player,player->x+ 12, player->y+ 16)){
            player->y+= 4;
            moveGameSprites(player, player->x, player->y);
        }
    }else{
        if (couldMove(player,player->x, player->y + 16) && couldMove(player,player->x+ 8, player->y+ 16)){
            player->y+= 4;
            moveGameSprites(player, player->x, player->y);
        }
    }
}          
/*
Процедура атаки
Вызывается после нажатия кнопки В во время уровня. В зависимости от того, куда повернут игрок, шляпа выстреливает
вправо или влево. Верхние спрайты игрока изменяются, шляпа с игрока пропадает.
Если на пути шляпы появляется враг - он погибает, шляпа пропадает. Если препятствие - шляпа
врезается в него. После того как шляпа окончила своей полет, верхние спрайты игрока снова меняются
и там снова появляется шляпа
*/
void shot(struct object* player){
    if (shooting== 0){
        if (flipFlag==0){
            set_sprite_prop(5, S_FLIPX);
            set_sprite_prop(6, S_FLIPX);
            player->spriteID[0]= 6;
            player->spriteID[1]= 5;
            currentSpeedX= -4;
        }else {
            player->spriteID[0]= 5;
            player->spriteID[1]= 6;
            currentSpeedX= 4;
        }
        move_sprite(1, 0, 0);
        move_sprite(2, 0, 0);
        moveGameSprites(player, player->x, player->y);
        hat.x= player->x;
        hat.y= player->y; 
        hat.spriteID[0]= 7;
        hat.spriteID[1]= 0;
        shooting= 1;   
    }
    hat.x+= currentSpeedX;
        if (!(couldMove(&hat, hat.x+ 4, hat.y)) || (hat.x<=8) || (hat.x>= 160) 
        || (onespritecollision(&hat, &j1)) || (onespritecollision(&hat, &j2)) 
        || (onespritecollision(&hat, &j3)) || (onespritecollision(&hat, &j4))){
            set_sprite_prop(5, get_sprite_prop(0));
            set_sprite_prop(6, get_sprite_prop(0));
            move_sprite(5, 0, 0);
            move_sprite(6, 0, 0);
            if (flipFlag==0){
                set_sprite_prop(1, S_FLIPX);
                set_sprite_prop(2, S_FLIPX);
                player->spriteID[0]= 2;
                player->spriteID[1]= 1;

            }else{
                set_sprite_prop(1, get_sprite_prop(0));
                set_sprite_prop(2, get_sprite_prop(0));
                player->spriteID[0]= 1;
                player->spriteID[1]= 2;

            }
            moveGameSprites(player, player->x, player->y);
            shooting= 0;
            if (onespritecollision(&hat, &j1)){
                killin(&j1);
                hat.x= 0;
                hat.y= 0;
            }else if (onespritecollision(&hat, &j2))
            {
                killin(&j2);
                hat.x= 0;
                hat.y= 0;
            }else if (onespritecollision(&hat, &j3))
            {
                killin(&j3);
                hat.x= 0;
                hat.y= 0;
            }else if (onespritecollision(&hat, &j4))
            {
                killin(&j4);
                hat.x= 0;
                hat.y= 0;
            }
        }
        move_sprite(hat.spriteID[0], hat.x, hat.y);
}
/*
Процедура для анимации движения шипов по вертикали
Шип движется между двумя предельными точками, которые мы объявляли в одной из процедур выше
При достижении предельной точки изменяет направление своего движения на противоположное. Движение вверх вниз
*/
void spikesmovinver(struct object* spike1){
    if (spike1->status== 0){
        spike1->y+=4;
        if(spike1->y>= spike1->limits[3]){
            spike1->status= 1;;
        }
    }else{
        spike1->y-=4;
        if(spike1->y<= spike1->limits[1]){
            spike1->status= 0;
        }
    }
    move_sprite(spike1->spriteID[0], spike1->x, spike1->y); 
}
/*
Процедура для анимации движения шипов по горизонтали
Шип движется между двумя предельными точками, которые мы объявляли в одной из процедур выше
При достижении предельной точки изменяет направление своего движения на противоположное. Движение вправо влево
*/
void spikesmovinhor(struct object* spike1){
    if (spike1->status== 0){
        spike1->x+=4;
        if(spike1->x>= spike1->limits[2]){
            spike1->status= 1;;
        }
    }else{
        spike1->x-=4;
        if(spike1->x<= spike1->limits[0]){
            spike1->status= 0;
        }
    }
    move_sprite(spike1->spriteID[0], spike1->x, spike1->y);

}
//Аналогичная процедура для анимации движения медуз по горизонтали
void jellymovinhor(struct object* jelly){
    if (jelly->status== 0){
        jelly->x+=4;
        if(jelly->x>= jelly->limits[2]){
            jelly->status= 1;
        }
    }else{
        jelly->x-=4;
        if(jelly->x<= jelly->limits[0]){
            jelly->status= 0;
        }
    }
    moveGameSprites(jelly, jelly->x, jelly->y);
}
/*
Процедура прыжка, ее вызов происходит после нажатия кнопки А
Игрок начинает движение вверх до тех пор, пока не достигнет максимальной высоты либо не встретит препятствие
Аналогично с процедурой падения происходит проверка либо 2х либо 3х тайлов в зависимости от х-координаты игрока
*/
void weakjump(struct object* player){
    if (jumping== 0){
        currentSpeedY= 0;
        jumping= 1;
    }
    if (player->x%8 !=0){
        if (couldMove(player,player->x-4, player->y - 4) && couldMove(player,player->x+ 4, player->y- 4) &&(couldMove(player,player->x+ 12, player->y- 4)) && (currentSpeedY<=8)){
            player->y-= 4;
            moveGameSprites(player, player->x, player->y);
            currentSpeedY+=2;
        }else{
            jumping=0;
        }
    }else{
        if ((couldMove(player,player->x, player->y - 4) && couldMove(player,player->x+ 8, player->y- 4)) && (currentSpeedY<=8)){
            player->y-= 4;
            moveGameSprites(player, player->x, player->y);
            currentSpeedY+=2;
        }else{
            jumping=0;
        }
    }
}
/*
Функция клавиатуры
Проверяет, входят ли новые координаты курсора в границы клавиатуры
Возвращает 1 если курсор можно передвинуть на новые координаты, иначе 0
*/
UBYTE isWithinKeyboard(UINT8 x, UINT8 y){
    // Исключения для двух клавиш в самом низу клавиатуры
    if(x==140 && y==144 || x==156 && y==144){
        return 1;
    }
    return x >= mincursorx && x <= maxcursorx && y >= mincursory && y <= maxcursory;
}
/*
Процедура клавиатуры
Добавляет выбранный символ в имя персонажа, пока не достигнут предел по символам
*/
void addtoplayername(struct coursor* cur){
    // work out index of select character in charactermap
    UINT8 characterindex = cur->row * 10 + cur->col + 1+ 51; // add one as space is first character in sprites
    if(namecharacterindex == 13) return; // max name length reached
    playername[namecharacterindex] = characterindex;
    namecharacterindex++;
}
/*
Процедура клавиатуры
Удаляет символ из имени
*/
void removefromplayername(){
    if(namecharacterindex>0){
        namecharacterindex--;
        playername[namecharacterindex] = 81;// replace with space
    }
}
/*
Процедура клавиатуры
Отрисовывает имя игрока во время ввода
Вызывается при изменении имени
*/
void drawplayername(){
    set_bkg_tiles(1, 4, 13, 1, playername);
}
/*
Процедура клавиатуры
В зависимости от положения курсора после нажатия кнопки А добавляет или удаляет символ из имени героя
либо же завершает ввод
*/
void updateplayername(struct coursor* cursor){
    // check if cursor at delete or done
    if(cursor->col==8 && cursor->row == 4){
        // delete
        removefromplayername();
        drawplayername();
    }
    else if (cursor->col==9 && cursor->row == 4){
        // player finished
        playerhasname = 1;
    }
    else{
        addtoplayername(cursor);
        drawplayername();
    }
}
/*
Главная процедура клавиатуры, в которой собраны все предыдущие 
*/
void askname(){
    set_bkg_tiles(0, 0, 21, 18, keyboard);                  //загрузка экрана клавиатуры
    set_sprite_data(0, 14, shortile);
    set_sprite_tile(31, 13);
    scroll_bkg(4, 0);
    fadein();
    SHOW_BKG;
    DISPLAY_ON;
    SHOW_SPRITES;
    dot.x= 12;
    dot.y= 80;
    dot.col= 0;
    dot.row= 0;
    move_sprite(31, dot.x, dot.y);
     while(!playerhasname){                                //цикл работает, пока не завершен ввод
        if(keydown){                                       //ликвидирует зажатие кнопок
            waitpadup();                                   
            keydown = 0;
        }

        switch(joypad()){                                  //в зависимоти от того, какая кнопка направления нажата,     
            case J_UP:                                     //перемещает курсор
                if(isWithinKeyboard(dot.x, dot.y - 16)){
                    dot.y -= 16;
                    scroll_sprite(31,0,-16);
                    keydown = 1;
                    dot.row--;
                }
                break;
            case J_DOWN: 
                if(isWithinKeyboard(dot.x, dot.y + 16)){            
                    dot.y += 16;
                    scroll_sprite(31,0,16);
                    keydown = 1;
                    dot.row++;
                }
                break;  
            case J_LEFT: 
                if(isWithinKeyboard(dot.x - 16, dot.y)){
                    dot.x -= 16;
                    scroll_sprite(31,-16,0);
                    keydown = 1;
                    dot.col--;
                }
                break; 
            case J_RIGHT: 
                if(isWithinKeyboard(dot.x + 16, dot.y)){            
                    dot.x += 16;
                    scroll_sprite(31,16,0);
                    keydown = 1;
                    dot.col++;
                }
                break;
            case J_A:                   //После нажатия А в зависимости от позиции курсора происходит сответствующая операция                     
                updateplayername(&dot);
                keydown = 1;                
                break;
        }
        perfDelay(2);
    }
    dot.x= 0;
    dot.y= 0;
    move_sprite(31, dot.x, dot.y);
    playerhasname= 0;
    namecharacterindex= 0;
    scroll_bkg(-4, 0);
}
/*
Данная процедура копирует массивы таблицы рекордов для ее удобного редактирования
*/
void arreq(){                 
    UINT8 i;
    for (i=0; i!=28; i++){
        resSCORE[i]= grscore[i];
    }
    for (i=0; i!=91; i++){
        resREC[i]= records[i];
    }
}
/*
Процедура, в которой происходит редактирование таблицы рекордов при внесении нового рекорда
*/
void newadd(UINT16 res){
    INT8 k, i, n;
        if ((res>=score[0] && res<score[1])){         //если рекорд больше первого, но меньше второго
            score[0]= res;
            k=23;
            for (i=27;i!=k; i--){                     //последняя строка массива спрайтов-очков заменяется
                n= res%10;                            //на новое число
                res=res/10;
                grscore[i]= 82+ n;                   
            }
            k= 91;
            n=0;
            for (i=78; i!=k; i++){                    //последняя строка массива спрайтов-имен заменяется
                records[i]= playername[n];            //на введенное имя
                n++;
            }

        }else if (res>=score[1] && res<score[2])      //если рекорд больше второго и меньше третьего
        {
            score[0]= score[1];
            score[1]= res;
            for (i=24; i!=28; i++){                   //перенос 3 строки массива спрайтов-очков на 4
                n=grscore[i-8];
                grscore[i]= n;
            }
            k=15;
            for (i=19; i!=k; i--){                    //перезапись в 3 строку массива спрайтов-очков
                n= res%10;
                res=res/10;
                grscore[i]= 82+ n;
            }
            for (i=78; i!=91; i++){                   //перенос 3 строки массива спрайтов-имен на 4
                n=records[i-26];
                records[i]= n;
            }
            k=65;
            n=0;
            for (i=52;i!=k;i++){                      //перезапись в 3 строку массива спрайтов-имен
                records[i]= playername[n];
                playername[n]= 81;
                n++;
            }

        }else if (res>=score[2] && res<score[3])      //если рекорд больше или равен третьему и меньше четвертого
        {
            score[0]= score[1];
            score[1]= score[2];
            score[2]= res; 
            arreq();                                  //копирование массива спрайтов-очков и спрайтов-имен
            k=65;
            for (i=26;i!=k; i++){                     //перенос спрайтов-имен на одну строку вниз
                n= resREC[i];
                records[i+26]= n;
            }
            k=20;
            for (i=8;i!=k; i++){                      //перенос спрайтов-очков на одну строку вниз
                n= resSCORE[i];
                grscore[i+8]= n;
            }
            k=39;
            n=0;
            for (i=26; i!=k; i++){                    //запись нового имени в массив спрайтов-имен
                records[i]= playername[n];
                playername[n]= 81;
                n++;
            }
            k=7;
            for (i=11; i!=k; i--){                    //запись нового числа очков в массив спрайтов-очков
                n= res%10;
                res=res/10;
                grscore[i]= 82+ n;
            }
        }else if (res>=score[3])                      //если рекорд больше четвертого рекорда
        {
            score[0]= score[1];
            score[1]= score[2];
            score[2]= score[3]; 
            score[3]= res;
            arreq();                                  //копирование массива спрайтов-очков и спрайтов-имен
            k=65;
            for (i=0;i!=k; i++){                      //освобождение первой строки в массиве спрайтов-имен
                n= resREC[i];                         //для нового имени, перенос всех имен на строку вниз
                records[i+26]= n;
            }
            k=20;
            for (i=0;i!=k; i++){                      //аналогичные процессы но для массива спрайтов-очков
                n= resSCORE[i];
                grscore[i+8]= n;
            }
            k=13;
            n=0;
            for (i=0; i!=k; i++){                     //перезапись в массив спрайтов-имен
                records[i]= playername[n];
                playername[n]= 81;
                n++;
            }
            k=-1;
            for (i=3; i!=k; i--){                     //перезапись в массив спрайтов-очков
                n= res%10;
                res=res/10;
                grscore[i]= 82+ n;
            }
        }
}
/*
Данная процедура отображает таблицу рекордов
*/
void showstats(){
    HIDE_SPRITES;
    set_bkg_tiles(0, 0, 21, 18, rec);
    set_bkg_tiles(1, 6, 13, 7, records);
    set_bkg_tiles(15, 6, 4, 7, grscore);
    scroll_bkg(4, 0);
    fadein();
    SHOW_BKG;
    perfDelay(580);
    fadeout();
    scroll_bkg(-4, 0);
}
/*
Данная процедура отображает начальный экран до тех пор пока пользователь не нажмет кнопку START
*/
void splashScreen(){
    BGP_REG= 0xFF;
    SHOW_BKG;
    DISPLAY_ON;
    set_bkg_data(0, 254, starstar_data);
    set_bkg_tiles(0, 0, 20, 18, starstar_map);
    perfDelay(300);
    fadein();
    while (joypad() != J_START){
    }
    fadeout();
}
/*
Единая Функция, где проверяются столкновения игрока с врагами и движущимися преградами
При любом столкновении возвращает 1, иначе 0
*/
BYTE grandCollision(){
    return((onespritecollision(&sp1, &star)) || (onespritecollision(&sp2, &star)) || (onespritecollision(&sp3, &star)) || (collision(&star, &j1)) || (collision(&star, &j2)) || (collision(&star, &j3)) || (collision(&star, &j4)));
}
/*
Эта процедура скрывает от пользователя спрайты, перемещает их из зоны видимости за границы экрана
*/
void surfaceCleaning(){
    UINT8 i;
    for (i=1; i!=27; i++){
        move_sprite(i, 0, 0);
    }
}
/*
Процедура, в которой работает механика уровня
*/
void level(){
    UBYTE i;
    flipFlag= 1;
    shooting= 0;
    jumping= 0;
    lifes= 3;
    points= 0;
    exitAM= 0;
    for (i=0; i!=5; i++){                                       //обнуление массива собранных монет
        coinsAmount[i]= 0;
    }
    set_bkg_tiles(0, 0, 20, 18, levelMap);                      //Загрузка изображения уровня
    starrecover();
    setStar();
    setSpikes(&sp1, 8, 1, 72, 32, 72, 80);                      //внесение параметров для движущихся шипов
    setSpikes(&sp2, 9, 0, 96, 56, 144, 56);
    setSpikes(&sp3, 10, 1, 32, 104, 88, 104);
    setJelly(&j1, 11, 12, 13, 14, 0, 128, 32, 0, 0);            //внесение параметров для медуз
    setJelly(&j2, 15, 16, 17, 18, 0, 8, 80, 0, 0);
    setJelly(&j3, 19, 20, 21, 22, 0, 144, 72, 0, 0);
    setJelly(&j4, 23, 24, 25, 26, 1, 40, 112, 136, 112);
    SHOW_BKG;
    fadein();
    SHOW_SPRITES;
    while ( (lifes!= 0) && !(exitAM)){                          //цикл идет пока есть жизни либо игрок не дошел до финиша
        if (((joypad() & J_A)                                   //прыгать можно лишь когда под игроком есть платформы
        && !(couldMove(&star, star.x, star.y + 16) 
        && couldMove(&star,star.x+ 8, star.y+ 16))) 
        || (jumping== 1)){
            weakjump(&star);
        }
        if (joypad() & J_B || (shooting== 1)){                  //после нажатия B запускается процедура атаки
            shot(&star);
        }
        if ((joypad() & J_LEFT) && (star.x!=8)){                //двигаться влево можно только в границах экрана
            if (flipFlag== 1){                                  //происходит отражение игрока по горизонтали
                reverse(&star);                                 //если ранее игрок двигался вправо
                flipFlag= 0;
            }
            if (star.y% 8!= 0){                                 //во время падения игрок может находиться не ровно в 
                if (couldMove(&star,star.x - 8, star.y-4)       //четырех спрайтах. Он может находиться в 6 спрайтах
                && couldMove(&star,star.x - 8, star.y+ 4)       //тогда при движении вправо и влево нужно 
                && couldMove(&star,star.x - 8, star.y +12)){    //проверять на проходимость уже не 2, а 3 спрайта
                    star.x-= 4;
                    moveGameSprites(&star, star.x, star.y);
                }
            }else{
                if (couldMove(&star,star.x - 4, star.y) 
                && couldMove(&star,star.x - 4, star.y+8)){
                    star.x-= 4;
                    moveGameSprites(&star, star.x, star.y);
                }
            }
        }
        if ((joypad() & J_RIGHT) && (star.x!=152)){             //все аналогично движению влево
            if (flipFlag== 0){
                reverse(&star);
                flipFlag= 1;
            }
            if (star.y% 8!= 0){
                if (couldMove(&star,star.x + 16, star.y- 4) 
                && couldMove(&star,star.x + 16, star.y+ 4) 
                && couldMove(&star,star.x + 16, star.y + 12)){
                    star.x+= 4;
                    moveGameSprites(&star, star.x, star.y);
                }
            }else{
                if (couldMove(&star,star.x + 16, star.y) 
                && couldMove(&star,star.x + 16, star.y+8)){
                    star.x+= 4;
                    moveGameSprites(&star, star.x, star.y);
                }
            }    
        }
        if (jumping== 0){                         //если игрок закончил прыжок, начинается падение
            falling(&star);
        }
        spikesmovinver(&sp1);                     //движение шипов
        spikesmovinhor(&sp2);
        spikesmovinhor(&sp3);
        if (j4.x!= 160+16){                       //движение медузы, если она не убита
            jellymovinhor(&j4);
        }
        perfDelay(3);                             //маленькая задержка для комфортной игры
        if ((grandCollision()) || star.y>144){    //если игрок упадет за край экрана либо столкнется с врагом
            death();                              //запускается процедура смерти
        }
    }
    points+= lifes* 1000;                         //Очки увеличиваются на тысячу за каждую оставшуюся жизнь
    HIDE_SPRITES;
    surfaceCleaning();                            //чистка экрана от спрайтов перед попаданием в меню
    fadeout();                                    //затемнение
}
/*
Функция меню
Если возвращает 1 - начинается уровень
                0 - показывает таблицу рекордов
*/
UBYTE menu(){
    UBYTE i;
    UINT8 arr[1]= {0x51};
    UINT8 orr[1]= {0x60};
    BGP_REG= 0xFF;
    set_bkg_tiles(0, 0, 21, 18, menuMAP);               //Загрузка изображения меню
    scroll_bkg(4, 0);
    set_bkg_tiles(1, 9, 1, 1, arr);
    set_bkg_tiles(1, 6, 1, 1, orr);
    SHOW_BKG;
    HIDE_SPRITES;
    fadein();
    i=1;
    while (joypad() != J_A){                            //цикл идет пока пользователь не выберет что-нибудь
        if ((joypad() & J_DOWN) || (joypad() & J_UP)){  //при нажатии вверх/вниз происходит анимация 
            if (i==0){                                  //выбора одной из опций
                set_bkg_tiles(1, 9, 1, 1, arr);
                set_bkg_tiles(1, 6, 1, 1, orr);
                i=1;
            }else{
                set_bkg_tiles(1, 9, 1, 1, orr);
                set_bkg_tiles(1, 6, 1, 1, arr);
                i=0;
            }
            waitpadup();                                //убирает возможность зажатия кнопок
        }
        perfDelay(5);                                   //небольшая задержка для замедления смены кадров
    }
    fadeout();                                          //затемнение
    scroll_bkg(-4, 0);
    return i;                                           
}
/*
Главная процедура, управляющая работой всей игры
*/
void cleaning(){
    UINT8 i;
    if (!(grscore[3]>81 && grscore[3]<92)){
        for (i=0; i!=91; i++){
            records[i]= 81;
        }
        for (i=0; i!=4; i++){
            score[i]=0;
        }
        for (i= 0; i!=28; i++){
            grscore[i]= 81;
        }
    }
}
void gameprocess(){
    splashScreen();                         //Запускает начальный экран
    cleaning();                             //При первом запуске заполняет таблицу рекордов пустыми значениями
    set_bkg_data(0, 97, FInal);             //загружает в память консоли тайлы фона
    set_sprite_data(0, 15, shortile);       //загружает в память консоли спрайты
    while (1){                              
        if (menu()){                        //В зависимости от выбора в меню начинается уровень либо 
            level();                        //включается таблица рекордов
            if (points>score[0]){           //Если число очков после уровня больше минимального рекорда 
                askname();                  //То пользователю предлагают ввести имя
                newadd(points);             //Рекорд добавляется в таблицу
                showstats();                //Пользователь видит таблицу рекордов со своим результатом
                perfDelay(500);             //Долгая задержка
            }
        }else{
            showstats();
        }
    }
}

void main(){
    ENABLE_RAM_MBC1;
    gameprocess();
    DISABLE_RAM_MBC1;
}
Lift_leveleditor.exe

L�ser filen level.bmp och hittar alla svarta pixlar och g�r en lista av dess koordinater.
Mellan dessa koordinatpunkter drags str�ck, vilket kommer att vara terr�ngmarken p� banan.
Programmet kommer att dra en linje fr�n punkten l�ngst till v�nster till n�rmsta punkt.
Och sedan fr�n denna punkt dra en linje till n�rmsta n�sta punkt.

Vid start v�ljs ett v�rde "Cut-off" f�r hur l�ngt avst�nd mellan 2 punkter som inte ska bli
ihopkopplade. Beh�vs f�r att g�ra fria strukturer som inte �r kopplade till bottenlagret.
Detta kommer att skriva in "BREAK" mellan tv� punkter i "level.txt".
Dessa fria strukturer kommer att l�nkas fr�n f�rsta punkt till sista. Den enda struktur som inte
kommer att l�nkas �r den f�rsta som hittas, vilket kommer att representera bottenlagret.

N�r Lift_leveleditor.exe �r klart kommer den att skriva en level.txt fil med dessa koordinater.
Var startpositionen f�r spelaren ska vara kan v�ljas genom att �ndra koordinaten f�r "playerpos".
F�r att �ndra storleken p� banan kan "scale_factor" �ndras fr�n 1 1 till andra v�rden f�r att
multiplicera kordinaterna med respektive faktor.



EXTRA?
l�gg in ljud i kod?

fler banor?

fler terrain textures?

fiende motor ljud?
mship auto landing complete?
mship takeoff begin/canceled?
col ljud i meny?
col ljud f�r obj?


BUGGAR?

krash n�r textruta visades samtidigt som ship i input box (skepp dockat till scanner ner i mship f�rst)

bugg krash efter ship upgrade efter pant av vapen...

KANSKE f�r att worldptr i weapon ej updaterades vid v�rldsbyte, fixad nu

vid fire..1234 (n�r ny kropp skapas) krash
h�nder n�r projektil skapas
kanske n�r den skapas inuti annan kropp
krash �ven vid 9 och senare
terrain f�rsvan precis innan? ritades ej

n�r rep f�lldes ut, l�g object i inputbox

krash vid fire, p� 9, spakade fixture, kanske inuti annan kropp och skapade collision? tags bort?



F�rslag

?b�rja ryka om trasig?
?ta bort cam gear?
?fiender ska kunna �ndra target, tex om tar skada?
?static lib f�r oggvorbis?
?planet names?
?fiender spawnas utanf�r terr�ng?
?gradient skyline? ljusare n�ra ytan?

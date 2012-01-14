inherit ROOM;

void setup( void ) {
   add_area( "Gurba" );
   add_area( "BorderWalk" );
   add_area( "BorderWalkNorth" );
   add_area( "GurbaOutdoor" );
   
   set_brief( "Border Walk - North" );
   set_long( "You are on the Border Walk, the long and narrow street which surrounds the town of Gurba in a large rectangle. It runs along the inside edge of the stone town wall, which at the moment lies to your north." );
   
   set_exits( ([ 
		  "west" : "/domains/gurba/rooms/border_north9",
		  "east" : "/domains/gurba/rooms/border_northeast",
		  ]) );
   
}

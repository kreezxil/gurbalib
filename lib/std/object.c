#include <type.h>
inherit "/std/body/size";	/* satch */
inherit "/std/modules/m_autoload_filename";

static string short_desc;
string long_desc;
static object object_environment;
static string *ids;
static string *adjs;
static int object_size;
static mapping object_commands;
static string detailed_desc;
private int configured;
static int eatable;
static int gettable;
static int object_value;
static int weight;

void setup();

void create(void) {
   string tmp;

   if (configured++) {
      return;
   }

   short_desc = "";
   ids = ( { "nondescript thing" } );
   adjs = ( { } );

   tmp = file_name();
   set_autoload_filename(tmp);

   if (!clone_num()) {
      OBJECT_D->register_object(this_object());
   } else {
      OBJECT_D->register_clone(this_object());
   }
}

void set_id(string str, varargs mixed args ...) {
   int i, max;

   if (sizeof(args) && typeof(args[0]) == T_ARRAY) {
      args = args[0];
   }

   ids = ( { str } );
   max = sizeof(args);
   for (i = 0; i < max; i++) {
      ids += ( { (string) args[i] } );
   }
}

void add_id(string str, varargs mixed args ...) {
   int size;

   ids -= ( { "nondescript thing" } );
   if (sizeof(args) && typeof(args[0]) == T_ARRAY) {
      args = args[0];
   }
   size = sizeof(ids);
   /* fix args to set_id so that ids[0] preserved after adding new ids */
   switch (size) {
      case 0:
	 set_id(str, args);
	 break;
      case 1:
	 set_id(ids[0], ( { str } ) | args);
	 break;
      default:
	 set_id(ids[0], ( { str } ) | args | ids[1..(size - 1)]);
   }
}

void add_ids(string str, varargs mixed args ...) {
   add_id(str, args);
}

string query_id(void) {
   return ids[0];
}

string *query_ids(void) {
   return ids;
}

int is_id(string id) {
   int i, max;

   max = sizeof(ids);
   for (i = 0; i < max; i++) {
      if (ids[i] == id) {
	 return 1;
      }
   }
   return 0;
}

void set_adj(string str, varargs mixed args ...) {
   int i, max;

   adjs = ( { str } );
   max = sizeof(args);
   for (i = 0; i < max; i++) {
      adjs |= ( { (string) args[i] } );
   }
}

void add_adj(string str, varargs mixed args ...) {
   int i, max;

   if (!adjs) {
      adjs = ( { } );
   }
   adjs |= ( { str } );
   max = sizeof(args);
   for (i = 0; i < max; i++) {
      adjs |= ( { (string) args[i] } );
   }
}

string *query_adjs(void) {
   return adjs;
}

string query_adj(void) {
   if (sizeof(adjs) == 0) {
      return "";
   }
   return adjs[0];
}

int is_adj(string adj) {
   int i, max;

   max = sizeof(adjs);
   for (i = 0; i < max; i++) {
      if (adjs[i] == adj)
	 return 1;
   }
   return 0;
}

void set_short(string str) {
   short_desc = str;
}

string query_short(void) {
   if (!short_desc || short_desc == "") {
      if (!query_adj() || query_adj() == "") {
	 short_desc = article(query_id()) + " " + query_id();
      } else {
	 short_desc = article(query_adj()) + " " + query_adj() + " "
	    + query_id();
      }
   }
   return short_desc;
}

void set_long(string str) {
   long_desc = str;
}

string query_long(void) {
   return long_desc;
}

object query_environment(void) {
   return object_environment;
}

void set_environment(object ob) {
   console_msg("set_environment() : " + dump_value(ob, ([])) + "\n");
   object_environment = ob;
}

void destruct(void) {
   if (object_environment)
      object_environment->remove_object(this_object());

   if (clone_num() == 0) {
      OBJECT_D->unregister_object(this_object());
   } else {
      OBJECT_D->unregister_clone(this_object());
   }
   destruct_object(this_object());
}

/* This is a place holder for doing an action before a move
   See domains/gurba/objects/bell.c for an example 
   XXX Are these really needed????  */
int pre_move() {
   return 1;
}

/* This is a place holder for doing an action after a move. 
   See domains/gurba/objects/bell.c for an example 
   Note: we don't return a value here because the move was already successful
   Were just cleaning up and don't care if post_move works or not.
*/
void post_move() {
}

nomask int move(mixed destination) {
   mixed err;
   object dest, curr;

   if (!destination) return 0;

   if (!pre_move()) return 1;
   
/* XXX Needs review.... work */
   curr = this_environment();
   if (curr && this_object()->is_living()) {
      curr->event("body_leave", this_object());
   }

   if (typeof(destination) == T_STRING) {
      /* Remove trailing .c */
      if (destination[strlen(destination) - 2..] == ".c")
	 destination = destination[..strlen(destination) - 3];
      dest = find_object(destination);
      if (!dest) {
         catch {
            dest = compile_object(destination);
            dest->setup();
         } : {
            dest = nil;
         }
      }
   } else dest = destination;

   if (!dest) return 0;
   if (object_environment == dest) return 0;

   err = dest->receive_object(this_object());

   if (!err || (err != 1)) {
      if (err) write("Error in move:" + err + "\n");
      return 0;
   }

   if (object_environment) {
      object_environment->remove_object(this_object());
   }

   object_environment = dest;

   /* XXX Needs review.... work */
   if (this_object()->is_living()) {
      dest->event("body_enter", this_object());
   }

   post_move();
   return 1;
}

nomask void add_verb_rules(string verb, varargs mixed rules) {
   switch (typeof(rules)) {
      case T_NIL:
	 PARSE_D->add_object_rules(( { verb, ""} ));
	 break;
      case T_STRING:
	 PARSE_D->add_object_rules(( { verb, rules} ));
	 break;
      case T_ARRAY:
	 PARSE_D->add_object_rules(( { verb} ) + rules);
	 break;
   }
   return;
}

nomask mapping query_verb_rules() {
   return PARSE_D->query_objects_rules(this_object());
}

void add_object_command(string command, string func) {
   if (!object_commands) object_commands = ([ ]);
   if (!object_commands[command]) {
      object_commands += ([command:func]);
   } else {
      object_commands[command] = func;
   }
}

void remove_object_command(string command) {
   object_commands[command] = nil;
}

string query_object_command(string command) {
   if (!object_commands) object_commands = ([ ]);
   return object_commands[command];
}

string *query_object_commands(void) {
   string *keys;

   if (!object_commands) {
      object_commands = ([ ]);
   }
   keys = map_indices(object_commands);

   return keys;
}

void set_eatable(int value) {
   eatable = value;
}

int is_eatable() {
   return eatable;
}

void set_detailed_desc(string desc) {
   detailed_desc = desc;
}

string query_detailed_desc(void) {
   return detailed_desc;
}

void upgraded() {
   if (clone_num() != 0) {
      setup();
   }
}

void set_value(int val) {
   object_value = val;
}

int query_value(void) {
   return object_value;
}

void set_weight(int val) {
   weight = val;
}

int query_weight(void) {
   return weight;
}

int is_gettable(void) {
   return gettable;
}

void set_gettable(int get) {
   gettable = get;
}


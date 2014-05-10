#include <type.h>

inherit ob "/std/object";

static object *inventory;
static mapping inv_map;
static int internal_weight;
static int internal_max_weight;

void create(void) {
   ob::create();
   inventory = nil;
   inv_map = ([]);
}

void set_internal_max_weight(int x) {
   internal_max_weight = x;
}

void set_internal_weight(int x) {
	internal_weight = x;
}

int query_internal_max_weight() {
   return internal_max_weight;
}

int query_internal_weight() {
   return internal_weight;
}

int query_weight() {
   int x;

   x =::query_weight();
   if (internal_max_weight) {
      return x + internal_weight;
   }
   return x;
}

int is_container(void) {
   return 1;
}

private int baneful(object aggressor, object victim) {
	object *inv;
	string *aggressor_banes;
	int     i, dim;

	aggressor_banes = aggressor->query_banes();
	if (victim->query_baneful(aggressor_banes)) {
		return 1;
	}

	inv = victim->query_inventory();
	for (i = 0, dim = sizeof(inv); i < dim; i++) {
		if (inv[i]->query_baneful(aggressor_banes)) {
			return 1;
		}
	}

	return 0;
}

void object_arrived(object obj) {
   int amount, x;
   string type;
   object *inv;

   if (obj->is_money() && this_object()->is_player()) {
      amount = obj->query_amount();
      type = obj->query_currency();
      this_object()->add_money(type, amount);
      obj->destruct();
      return;
   }

   /* Player moves into room */
   if (obj->is_player()) {
      inv = this_object()->query_inventory();

      for (x = sizeof(inv) - 1; x >= 0; x--) {
         if (inv[x]->query_aggressive() && !baneful(inv[x], obj)) {
            inv[x]->attack(obj);
         }
      }
   }

   /* Aggressive monster moves into room */
   if (obj->query_aggressive()) {
      inv = this_object()->query_inventory();

      for (x = sizeof(inv) - 1; x >= 0; x--) {
         if (inv[x]->is_player() && !baneful(obj, inv[x])) {
            obj->attack(inv[x]);
         }
      }
   }
}

void object_removed(object obj) {
}

int receive_object(object "/std/object" obj) {
   int tmp;

   if (!inv_map) {
      inv_map = ([]);
   }

   inv_map[obj] = time();
   object_arrived(obj);
   return 1;
}

int remove_object(object obj) {
   if (!inv_map) {
      return 0;
   } else {
      inv_map[obj] = nil;
   }

   object_removed(obj);
   return 1;
}

nomask object *query_inventory(void) {
   if (inv_map) {
      return map_indices(inv_map);
   }
   return ({ });
}

object find_object_num(string name, int num) {
   int i, j;
   string *ids;
   object *inv;

   inv = query_inventory();

   if (inv) {
      for (i = 0; i < sizeof(inv); i++) {
         ids = inv[i]->query_ids();
         if (ids) {
            for (j = 0; j < sizeof(ids); j++) {
               if (lowercase(ids[j]) == lowercase(name)) {
                  num--;
                  if (num == 0) {
                     return (inv[i]);
                  }
               }
            }
         }
      }
   }
}

object find_object_filename(string name) {
   int i, max;
   object *inv;
   string tmp;

   inv = query_inventory();
   max = sizeof(inv);
   for (i = 0; i < max; i++) {
      tmp = inv[i]->base_name() + ".c";
      if (tmp == name) {
         return inv[i];
      }
   }
}

object find_adj_object_num(string adj, string name, int num) {
   int i, j, k;
   string *ids, *adjs;
   object *inv;

   inv = query_inventory();

   for (i = 0; i < sizeof(inv); i++) {
      ids = inv[i]->query_ids();
      if (ids) {
         for (j = 0; j < sizeof(ids); j++) {
            if (lowercase(ids[j]) == lowercase(name)) {
               if (inv[i]->is_adj(adj) == 1) {
                  num--;
                  if (num == 0) {
                     return inv[i];
                  }
               }
            }
         }
      }
   }
   return nil;
}

object find_adj_object(string adj, string name) {
   return find_adj_object_num(adj, name, 1);
}

object find_adjs_object_num(string * adj, string name, int num) {
   int i, j, k;
   string *ids, *adjs;
   object *inv;

   inv = query_inventory();

   for (i = 0; i < sizeof(inv); i++) {
      ids = inv[i]->query_ids();
      if (ids) {
         for (j = 0; j < sizeof(ids); j++) {
            if (lowercase(ids[j]) == lowercase(name)) {
               int nFound;
               nFound = 1;
               for (k = 0; k < sizeof(adj); k++) {
                  if (!inv[i]->is_adj(adj[k])) {
                     nFound = 0;
                  }
               }
               if (nFound) {
                  num--;
                  if (num == 0) {
                     return inv[i];
                  }
               }
            }
         }
      }
   }
   return nil;
}

object find_adjs_object(string * adj, string name) {
   return find_adjs_object_num(adj, name, 1);
}

object present(string name) {
   string what;
   int which;
   if (file_exists(name)) {
      return find_object_filename(name);
   } else if (file_exists(name + ".c")) {
      return find_object_filename(name + ".c");
   }
   if (sscanf(name, "%s %d", what, which) == 2) {
      return find_object_num(what, which);
   }
   return find_object_num(name, 1);
}

void error_loading_object(string name) {
   object ob;
   string filename;

   LOG_D->write_log("rooms", "Error: " + this_object()->base_name() +
      " loading object:" + name + "\n");

   filename = DOMAINS_DIR + "/required/objects/sing.c";
   ob = clone_object(filename);
   if (ob) {
      ob->move(object_name(this_object()));
      ob->setup();
   } else {
      LOG_D->write_log("rooms", "Error: " + this_object()->base_name() +
         " loading object:" + filename + "\n");
   }
}

void add_object(string filename, varargs int just_one) {
   object obj;
   if (empty_str(filename)) {
      return;
   }
   obj = clone_object(filename);
   if (obj) {
		if (just_one && present(filename)) {
			return;
		}
      obj->move(object_name(this_object()));
      obj->setup();
   } else {
      error_loading_object(filename);
   }
}

void set_objects(varargs string filename ...) {
   object ob;
   mapping obs;
   string name;
   int i, j;
   int num;
   object *inv;

   inv = query_inventory();
   obs = ([]);

   if (!filename || filename == ({ })) {
      return;
	}
   /* remove object number */
   for (i = 0; i < sizeof(filename); i++) {
      if (strstr(filename[i], "#") != -1) {
         name = filename[i][0..strstr(filename[i], "#") - 1];
      } else
         name = filename[i];
      /* Count number of name's in filename.  Store in mapping */
      if (!obs[name])
         obs[name] = 1;
      else
         obs[name] += 1;
   }

   filename = map_indices(obs);

   for (i = 0; i < sizeof(filename); i++) {
      num = obs[filename[i]];

      for (j = 0; j < sizeof(inv); j++) {
         if ((inv[j]->file_name() == filename[i]) ||
            (inv[j]->base_name() == filename[i])) {
            num--;
         } else {
            string tmp;
            tmp = this_object()->file_name() + "->set_objects(): " +
               filename[i] + "\n";
            LOG_D->write_log("container", "[" + ctime(time()) + "] " + tmp);
         }
      }

      while (num > 0) {
         add_object(filename[i]);
         num--;
      }
   }
}

void move_or_destruct_inventory() {
   object dst, *items;
   int i, sz;

   items = query_inventory();

   dst = this_object()->query_environment();
   sz = sizeof(items);

   for (i = 0; i < sz; i++) {
      if (!dst || (items[i]->move(dst) != 1)) {
         if (items[i]->is_player()) {
            items[i]->move(VOID);
         } else {
            items[i]->destruct();
         }
      }
   }
}

void upgraded() {
   int i, sz;

   if (inventory) {
      inventory -= ( {
         0, nil}
      );

      inv_map = ([]);

      for (i = 0, sz = sizeof(inventory); i < sz; i++) {
         inv_map[inventory[i]] = 1;
      }
      inventory = nil;
   }

   ::upgraded();
}

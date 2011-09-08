void usage() {
  write("Usage: wall [-h] MSG\n");
  write("Tell everyone on the mud: MSG.\n");
  write("Options:\n");
  write("\t-h\tHelp, this usage message.\n");
  write("\t-w\tRestrict the MSG to sending only to wizards, " +
    "not normal players.\n");
}

void main( string str ) {
  object *users;
  int i,max;

  if (!str || str == "") {
     usage();
     return;
  }
  if (sscanf(str, "-%s",str)) {
     usage();
     return;
  }

  if( !require_priv( "system" ) ) {
     write("You need admin permissions to do that.");
     return;
   }

  users = USER_D->query_users();
  max = sizeof(users);
  for (i = 0; i < max; i++) {
     users[i]->query_player()->message("WALL from " + 
        capitalize(this_player()->query_name()) + ": " + str + "\n");
  }
} 

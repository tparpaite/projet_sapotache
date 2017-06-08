
add_library(server STATIC server.c)
add_library(client SHARED client.c)
add_library(human SHARED human.c)
add_library(client_strategy SHARED client_strategy.c)
add_library(game STATIC game.c)
add_library(game_client STATIC game_client.c)
add_library(game_server STATIC game_server.c)




target_link_libraries(game_client game)
target_link_libraries(game_server game)
target_link_libraries(server game_server)
target_link_libraries(client_strategy game_client)
target_link_libraries(client game_client)
target_link_libraries(human game_client)

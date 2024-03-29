/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anon <anon@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/08 13:42:50 by jcueille          #+#    #+#             */
/*   Updated: 2022/10/03 14:53:02 by anon             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/includes.hpp"
#include "includes/parsing.hpp"
# include <sstream>

user *users = NULL;
channel *channels = NULL;
struct pollfd fds[SOMAXCONN];
int nfds = 1;

static int check_args(int ac, char **av)
{
	long int n;

	if (ac != 3)
		ft_exit("Wrong number of arguments\nHow to use: ./ircserv port password", 1, NULL);
	n = strtol(av[1], NULL, 10);
	if (n < 0 || n > 65535 || errno == ERANGE)
		ft_exit("Wrong port number.", errno == ERANGE ? errno : 1, NULL);
	return n;
}

int new_client(int id, struct pollfd *fd)
{
	struct s_user *tmp = users;
	struct s_user* new_user = new struct s_user();
	if (new_user == NULL)
		return -1;
	new_user->next = NULL;
	new_user->id = id;
	new_user->fd = fd->fd;
	new_user->idle = 0;
	time(&new_user->signon);
	new_user->nickname = "";
	new_user->realname = "";
	new_user->auth = 0;
	memset(new_user->modes, 0, sizeof(new_user->modes));
	new_user->hostname = "127.0.0.1";
	std::stringstream ss;
	ss << new_user->nickname << " id: " << new_user->id << " fd: " << new_user->fd;
	pp(std::string(RED), ss.str());
	if (!users)
	{
		users = new_user;
		new_user->modes[OPERATOR_MODE] = 1;
		new_user->prev = NULL;
	}
	else
	{
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = new_user;
		new_user->prev = tmp;
	}
	return 0;
	
}

void delete_client(user *u)
{
	if (users == u && u->next == NULL)
		users = NULL;
	else if (users == u && u->next)
		users = u->next;
	if (u->next)
		u->next->prev = u->prev;
	if (u->prev)
		u->prev->next = u->next;
	for (std::vector<channel *>::iterator it = u->channels.begin(); it != u->channels.end(); it++)
	{
		for (std::vector<user *>::iterator ite = (*it)->users.begin(); ite != (*it)->users.end(); ite++)
		{
			if ((*ite) == u)
			{
				send_to_all_chan(channel_message("PART " + (*it)->name + " " + "connection closed", u), (*it));
				(*it)->users.erase(ite);
				return ;
			}
		}
	}
	std::vector<channel *>().swap(u->channels);
	delete u;
}

void delete_channel(std::string name)
{
	channel * tmp = channels;
	while (tmp != NULL)
	{
		if (tmp->name == name)
		{
			tmp->next->prev = tmp->prev;
			tmp->prev->next = tmp->next;
			std::vector<user *>().swap(tmp->users);
			delete tmp;
			return ;
		}
		tmp = tmp->next;
	}

}

channel *new_channel(std::string name)
{
	channel *tmp = channels;
	channel* new_channel = new channel();
	if (new_channel == NULL)
		return NULL;
	new_channel->next = NULL;
	new_channel->name = name;
	if (!channels)
		channels = new_channel;
	else
	{
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = new_channel;
		new_channel->prev = tmp;
	}
	new_channel->key = "";
	new_channel->topic = ""; 
	new_channel->creation = current_time();
	memset(new_channel->modes, 0, sizeof(new_channel->modes));
	return new_channel;
	
}

void signalHandler( int signum ) {
   std::cout << "Interrupt signal (" << signum << ") received." << std::endl;
	ft_free_exit("SIGINT", SIGINT);

   exit(signum);  
}

int main (int argc, char *argv[])
{
	signal(SIGINT, signalHandler); 
	int port = check_args(argc, argv);

	int    len, rc, on = 1, new_client_id = 0;
	int    listen_sd = -1, new_sd = -1;
	int    end_server = FALSE, compress = FALSE, client_running = TRUE;
	int    close_conn;
	char   buffer[513];
	struct sockaddr_in6   addr;
	int    current_size = 0, i;

	/*************************************************************/
	/* Create an AF_INET6 stream socket to receive incoming      */
	/* connections on                                            */
	/*************************************************************/
	if ( (listen_sd = socket(AF_INET6, SOCK_STREAM, 0)) == -1)
		ft_exit(" socket creation failed.", errno, NULL);


	/*************************************************************/
	/* Allow socket descriptor to be reuseable                   */
	/*************************************************************/
	if ((rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
					(char*)&on, sizeof(on))) < 0)
	ft_exit(" setsockopt failed.", errno, &listen_sd);


	/*************************************************************/
	/* Set socket to be nonblocking. All of the sockets for      */
	/* the incoming connections will also be nonblocking since   */
	/* they will inherit that state from the listening socket.   */
	/*************************************************************/
	if ( (rc = ioctl(listen_sd, FIONBIO, (char*)&on)) < 0)
		ft_exit(" ioctl failed.", errno, &listen_sd);

	/*************************************************************/
	/* Bind the socket                                           */
	/*************************************************************/
	memset(&addr, 0, sizeof(addr));
	addr.sin6_family      = AF_INET6;
	memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
	addr.sin6_port        = htons(port);
	if ((rc = bind(listen_sd,
			(struct sockaddr *)&addr, sizeof(addr))) < 0)
		ft_exit(" bind failed.", errno, &listen_sd);

	/*************************************************************/
	/* Set the listen back log                                   */
	/*************************************************************/
	if ((rc = listen(listen_sd, SOMAXCONN)) < 0)
		ft_exit(" listen failed.", errno, &listen_sd);

	/*************************************************************/
	/* Initialize the pollfd structure                           */
	/*************************************************************/
	memset(fds, 0 , sizeof(fds));

	/*************************************************************/
	/* Set up the initial listening socket                        */
	/*************************************************************/
	fds[0].fd = listen_sd;
	fds[0].events = POLLIN;


	/*************************************************************/
	/* Loop waiting for incoming connects or for incoming data   */
	/* on any of the connected sockets.                          */
	/*************************************************************/
	//char astring[4000];

	do
	{
		/***********************************************************/
		/* Call poll() and wait 3 minutes for it to complete.      */
		/***********************************************************/
		//printf("Waiting on poll()...\n");
		if ((rc = poll(fds, nfds, -1)) < -1)
			ft_exit(" poll failed.", errno, &listen_sd);

		/***********************************************************/
		/* Check to see if the 3 minute time out expired.          */
		/***********************************************************/
		if (rc == 0)
		{
			printf("  poll() timed out.  End program.\n");
			break;
		}


		/***********************************************************/
		/* One or more descriptors are readable.  Need to          */
		/* determine which ones they are.                          */
		/***********************************************************/
		current_size = nfds;
		for (i = 0; i < current_size; i++)
		{
			if (DEBUG)
			{
				std::stringstream ss;
				ss << "nfds: " << nfds << "fd: " << fds[i].fd;
				pp(std::string(CYAN), ss.str());
			}
			/*********************************************************/
			/* Loop through to find the descriptors that returned    */
			/* POLLIN and determine whether it's the listening       */
			/* or the active connection.                             */
			/*********************************************************/
			if(fds[i].revents == 0)
				continue;

			/*********************************************************/
			/* If revents is not POLLIN, it's an unexpected result,  */
			/* log and end the server.                               */
			/*********************************************************/
			if(fds[i].revents != POLLIN)
			{
				printf("  Error! revents = %d\n", fds[i].revents);
				end_server = TRUE;
				break;
			}
			if (fds[i].fd == listen_sd)
			{
				/*******************************************************/
				/* Listening descriptor is readable.                   */
				/*******************************************************/
				printf("  Listening socket is readable\n");

				/*******************************************************/
				/* Accept all incoming connections that are            */
				/* queued up on the listening socket before we         */
				/* loop back and call poll again.                      */
				/*******************************************************/
				do
				{
					/*****************************************************/
					/* Accept each incoming connection. If               */
					/* accept fails with EWOULDBLOCK, then we            */
					/* have accepted all of them. Any other              */
					/* failure on accept will cause us to end the        */
					/* server.                                           */
					/*****************************************************/
					new_sd = accept(listen_sd, NULL, NULL);
					if (new_sd < 0)
					{
						if (errno != EWOULDBLOCK)
						{
							perror("  accept() failed");
							end_server = TRUE;
						}
						break;
					}

					/*****************************************************/
					/* Add the new incoming connection to the            */
					/* pollfd structure                                  */
					/*****************************************************/
					printf("  New incoming connection - %d\n", new_sd);
					fds[nfds].fd = new_sd;
					fds[nfds].events = POLLIN;
					if (new_client(new_client_id, &fds[nfds]) == -1)
						ft_free_exit(" user creation failed.", -1);
					nfds++;
					new_client_id++;

					/*****************************************************/
					/* Loop back up and accept another incoming          */
					/* connection                                        */
					/*****************************************************/
				} while (new_sd != -1);
			}

			/*********************************************************/
			/* This is not the listening socket, therefore an        */
			/* existing connection must be readable                  */
			/*********************************************************/

			else
			{
				//printf("  Descriptor %d is readable\n", fds[i].fd);
				close_conn = FALSE;
				/*******************************************************/
				/* Receive all incoming data on this socket            */
				/* before we loop back and call poll again.            */
				/*******************************************************/
				do
				{
					/*****************************************************/
					/* Receive data on this connection until the         */
					/* recv fails with EWOULDBLOCK. If any other         */
					/* failure occurs, we will close the                 */
					/* connection.                                       */
					/*****************************************************/

					memset(buffer, 0, sizeof(buffer));
					rc = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
					buffer[512] = '\0';
					if (rc < 0)
					{
						if (errno != EWOULDBLOCK)
						{
							perror("  recv() failed");
							close_conn = TRUE;
						}
						break;
					}

					/*****************************************************/
					/* Check to see if the connection has been           */
					/* closed by the client                              */
					/*****************************************************/
					if (rc == 0)
					{
						printf("  Connection closed\n");
						close_conn = TRUE;
						break;
					}
					/*****************************************************/
					/* Data was received                                 */
					/*****************************************************/
					len = rc;
					(void)len;
					//printf("  %d bytes received\n", len);
					if (DEBUG == 1)
					{
						std::stringstream ss;
    					ss << fds[i].fd;
						pp(std::string(CYAN), ss.str());
					}
					user *tmp_user = find_user_by_fd(fds[i].fd);
					if (DEBUG)
					{
						std::stringstream ss;
    					ss << fds[i].fd << " " << ( tmp_user ? tmp_user->nickname : "NULL");
						pp(std::string(CYAN), ss.str());
					}
					std::string cmd = buffer;
					std::stringstream ss(buffer);
					std::string token;
					
					while (std::getline(ss, token, '\n')) {
						Command tmp_cmd(token);
						if (DEBUG == 1)
						{
							pp(std::string(CYAN), tmp_user->nickname);
							print_user(tmp_user);
						}
						if (tmp_cmd.parse(fds, &nfds, tmp_user, argv[2]))
						{
							delete_client(tmp_user);
							close_conn = TRUE;
							break ;
						}
					}
					
					/*****************************************************/
					/* Echo the data back to the client                  */
					/*****************************************************/
					//rc = send(fds[i].fd, buffer, len, 0);
					if (rc < 0)
					{
						perror("  send() failed");
						close_conn = TRUE;
						break;
					}
					break;

				} while(client_running == TRUE);
				/*******************************************************/
				/* If the close_conn flag was turned on, we need       */
				/* to clean up this active connection. This            */
				/*clean up process includes removing the              */
				/* descriptor.                                         */
				/*******************************************************/
				if (close_conn)
				{
					if (DEBUG)
					{
						std::stringstream ss;
    					ss << "d:" << fds[i].fd;// << " " << ( tmp_user ? tmp_user->nickname : "NULL");
						pp(std::string(CYAN), ss.str());
					}
					delete_client(find_user_by_fd(fds[i].fd));
					close(fds[i].fd);
					fds[i].fd = -1;
					//current_size--;
					compress = TRUE;
				}


			}  /* End of existing connection is readable             */
		} /* End of loop through pollable descriptors              */

		/***********************************************************/
		/* If the compress flag was turned on, we need       */
		/* to squeeze together the array and decrement the number  */
		/* of file descriptors. We do not need to move back the    */
		/* events and revents fields because the events will always*/
		/* be POLLIN in this case, and revents is output.          */
		/***********************************************************/
		if (compress)
		{
			compress = FALSE;
			compress_array(fds, &nfds);
		}

	} while (end_server == FALSE); /* End of serving running.    */

	ft_free_exit("0", 0);

	

}

#include "ServerGroup.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

ServerGroup::ServerGroup()//TODO: add config file as parameter
{
	//TODO: get config file data and pass it to build function

}

ServerGroup::ServerGroup( const ServerGroup & src )
{

}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

ServerGroup::~ServerGroup()
{
}

std::string to_string(int n)
{
    std::ostringstream s;
    s << n;
    return s.str();
} 


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

ServerGroup &				ServerGroup::operator=( ServerGroup const & rhs )
{
	//if ( this != &rhs )
	//{
		//this->_value = rhs.getValue();
	//}
	return *this;
}

std::ostream &			operator<<( std::ostream & o, ServerGroup const & i )
{
	//o << "Value = " << i.getValue();
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/

void					ServerGroup::build()
{
	//TODO:  get_allserver_sockets, replace with config file parsing equivalent
	_portslist = Debugging.getServerPorts();
	_hostslist = Debugging.getServerHosts();
	//TODO:

	FD_ZERO(&_masterfds);
	FD_ZERO(&_masterwritefds);
	_fd_size = _hostslist.size();
	_servercount = _fd_size;
	_fd_cap = 0;

	//loop over all server_sockets create and  fill the _servers map
	for (int i = 0; i < _portslist.size(); i++)
	{
		Server		currentsrv;
		int			fd;
		currentsrv.setSocket(_hostslist[i], _portslist[i]);
		if (currentsrv.Create() != -1)
		{
			fd = currentsrv.getsocketfd();
			FD_SET(fd, &_masterfds);
			_servers_map.insert(std::make_pair(fd, currentsrv));
			
			_servers_vec.push_back(currentsrv);

			if (fd > _fd_cap)
				_fd_cap = fd;
		}
	}
	if (_fd_cap == 0)
		throw BuildException();
}

void					ServerGroup::start()
{
	struct timeval	timetostop;
	timetostop.tv_sec  = 1;
	timetostop.tv_usec = 0;
	


	std::fstream    body_file;
	size_t          body_size;
	HttpRequest http;
	std::ostringstream  body_stream;

	body_size = 0;
	body_file.open("body.txt", std::ios::out);

	while (true)
	{
		_readset = _masterfds;
		_writeset = _masterwritefds;
		if (select((int)_fd_cap + 1, &_readset, &_writeset, NULL, NULL) < 0)
			throw SelectException();

		for (size_t i = 0; i < _fd_cap + 1; i++)
		{
			
			if (FD_ISSET(i, &_writeset) || FD_ISSET(i, &_readset))
			{
				if (isServerFD(i)) //check if the server fd is the one wich is ready if true accept client connection
				{
					//accept connection
					int new_socket = acceptCon(i); 
					std::cout << "connection is accepted :" << new_socket << std::endl;
					FD_SET(new_socket, &_masterfds);
					if (new_socket > _fd_cap)
						_fd_cap = new_socket;
				}
				else
				{
					if (FD_ISSET(i, &_readset)) //coonection is to be read from
					{
						int flag;
						flag = http.handle_http_request(i, body_file, body_size, body_stream);
						if (!flag)
						{
							FD_CLR(i, & _masterfds);
							FD_SET(i, & _masterwritefds);
						}



		

						// if (s == "cgi")
						// {
						// 	int fd = open("test", O_RDWR | O_CREAT, 0777);
    					// 	int fork_id = fork();
							
						// }
					}
					else if (FD_ISSET(i, &_writeset)) // connection is ready to be written to
					{
						if (http.Get_Http_Method() == "POST")
						{
							body_file << body_stream.str() << std::endl;
							body_file.close();        
						}
						if (http.Get_Http_Method() == "POST" && http.get_value("Transfer-Encoding") == "chunked")
							http.handle_chunked_body();
						else if (http.Get_Http_Method() == "POST")
							http.handle_regular_body();
						Response 	ok;
						std::string	error_msg;

						ok.set_request_method(http.Get_Http_Method());
						ok.set_request_target(http.Get_Request_Target());
						ok.set_mybuffer(http.Get_Request_Target());
						ok.check_file();
						error_msg = ok.parsing_check();
						if (error_msg != "")
						{
							exit(0);
							ok.error_handling(error_msg);
						}
						else
						{	

						// 		std::string s;
						// Servers ok;
           				//  ok.parse_server("HTTP/conf");
						//  Conf my_config;
						//  Cgi pineb;

						//  //set env 
						 
						//  my_config = ok.get_server()[0];
						//  pineb = my_config.get_cgi();
						//  s = pineb.get_cgi_block_path();
						//  std::cout << s << std::endl;


						//  env 


							int fd = open("test", O_RDWR | O_CREAT, 0777);
							int fork_id = fork();
							if(fork_id == -1)
							{
								std::cout << "error on fork forking" << std::endl;
								exit(0); // fix me 
							}
							else if(fork_id == 0) // child process
							{
								// if R is GET
								// if R is POST dup file disc of body to input
								//pipe is limited because it can hang, and you dont need it since at the end you will put the output in a file.
									dup2(fd, 1);

									std::string cgi_location = "/usr/bin/python";
									std::string req_file = "script.py";  // 	ok.get_request_target();
								
									char *args[3];
									args[0] = (char *)cgi_location.c_str();  //cgi_location 
										args[1] = (char *)req_file.c_str();// PATH+  req_file
										args[2] = NULL;

									// https://www.cs.ait.ac.th/~on/O/oreilly/perl/perlnut/ch09_04.htm
									

									setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
									setenv("SERVER_SOFTWARE", "websert", 1);
									setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
									setenv("SERVER_PORT", std::to_string(8000).c_str(), 1);  // from config
									setenv("REQUEST_METHOD", "POST", 1);
									setenv("PATH_INFO", "/usr/bin/python", 1); // path script exmple /usr/bin/python /user/bin/cgi-php
									// setenv("PATH_TRANSLATED", file_path.c_str(), 1);
									setenv("QUERY_STRING", "empty=1&name=5", 1); // form request  request
									// setenv("DOCUMENT_ROOT", document_root.c_str(), 1);
									setenv("SCRIPT_NAME", "script.py", 1); // get from request request
									// setenv("REMOTE_HOST", remote_host.c_str(), 1);
									// setenv("REMOTE_ADDR", remote_address.c_str(), 1);
									setenv("CONTENT_TYPE", "", 1); // for //POST  an GET request
									setenv("CONTENT_LENGTH", std::to_string(13).c_str(), 1); // for post  request
									setenv("HTTP_USER_AGENT","gfx", 1);
									// setenv("HTTP_REFERER", referer.c_str(), 1);

									if (execve(args[0], args, NULL) == -1)
										perror("Could not execve fff");
								}
								else // parent process
								{
									int wstatus;
									waitpid(fork_id, &wstatus, 0);
									if (WIFEXITED(wstatus))
									{
										int status_code = WEXITSTATUS(wstatus);
										if (status_code != 0)
										{
											std::cout << "Failure with status code : " << status_code << std::endl;
											exit (1);
										}
									}

									int nbytes;
									char cgi_buff[1024] = {0};

									//lseek(fd, 0, SEEK_SET);
									while ((nbytes = read(fd, cgi_buff, 1024)) > 0)
									{
										std::cout << "cgi_biff ========================= "<< cgi_buff << std::endl;
										std::string c = "HTTP/1.1 200 OK\r\nServer: petitwebserv\r\nDate: 2102\r\nContent-type:text/html\r\nContent-Length: "+ std::to_string(nbytes) + "\n\r\n";
										std::string res = c + std::string(cgi_buff);
										ok.set_hello(res);


										std::cout << "Get HEllo :" << ok.get_hello()  << "=="<< ok.get_total_size() << std::endl;
						
 										break; 
									}
									close(fd);
								}

							// if (http.Get_Http_Method() == "GET")
							// 	body_size = ok.handle_Get_response();
							// else if (http.Get_Http_Method() == "DELETE")
							// 	ok.handle_delete_response(http.get_value("Connection"));
							// else if (http.Get_Http_Method() == "POST")
							// 	ok.handle_post_response(http.get_value("Connection"));							
						}
				
       									write(i , ok.get_hello() , ok.get_total_size());
									FD_CLR(i, & _masterwritefds);
       								close(i);
						// //write to connected socket
						// std::cout << "SENT RESPONSE\n";
						// char *hello = strdup("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 16\n\nNizaaaaaaaaar!!!");
        				// write(i , hello , strlen(hello));
						// //buufet 
						// FD_CLR(i, & _masterwritefds);

						// close(i);
					}
				}
			}
		}
	}
}

void					ServerGroup::stop()
{
	
}

/*
** --------------------------------- HANDELINGS ---------------------------------
*/
int		ServerGroup::acceptCon(int fd)
{
	int newsocket;
	struct sockaddr_in newaddr;
	unsigned int addrlen;
	
	newsocket = accept(fd , (struct sockaddr *)&newaddr, (socklen_t*)&addrlen);
	fcntl(newsocket, F_SETFL, O_NONBLOCK);

	if (newsocket < 0)
		throw AcceptException();
	_client_fds.push_back(newsocket);
	return newsocket;
}

int		ServerGroup::sendCon()
{
	return 0;
}
int		ServerGroup::recvCon()
{
	return 0;
}

/*
** --------------------------------- CHECKS ---------------------------------
*/

bool	ServerGroup::isServerFD(int fd)
{
	std::map<int, Server>::iterator it = _servers_map.find(fd);

	if (it == _servers_map.end())
		return false;
	else
		return true;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/


/* ************************************************************************** */
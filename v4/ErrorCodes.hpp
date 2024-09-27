// Numeric Replies: Error codes (based on RFC 1459)

// ERR_NOSUCHNICK (401): Used to indicate a nickname parameter is not recognized.
#define ERR_NOSUCHNICK 401

// ERR_NOSUCHSERVER (402): Used when a server parameter is not recognized.
#define ERR_NOSUCHSERVER 402

// ERR_NOSUCHCHANNEL (403): Used to indicate a channel parameter is not recognized.
#define ERR_NOSUCHCHANNEL 403

// ERR_CANNOTSENDTOCHAN (404): Used to indicate a message can't be sent to the channel.
#define ERR_CANNOTSENDTOCHAN 404

// ERR_TOOMANYCHANNELS (405): Indicates the user has joined too many channels.
#define ERR_TOOMANYCHANNELS 405

// ERR_NOORIGIN (409): Used when a PING or PONG message is missing the origin parameter.
#define ERR_NOORIGIN 409

// ERR_NORECIPIENT (411): No recipient given for a command that requires one.
#define ERR_NORECIPIENT 411

// ERR_NOTEXTTOSEND (412): No text provided for a message that expects text.
#define ERR_NOTEXTTOSEND 412

// ERR_UNKNOWNCOMMAND (421): Used when the command is not recognized.
#define ERR_UNKNOWNCOMMAND 421

// ERR_NONICKNAMEGIVEN (431): Used when no nickname is provided for NICK command.
#define ERR_NONICKNAMEGIVEN 431

// ERR_ERRONEUSNICKNAME (432): The given nickname is invalid or contains illegal characters.
#define ERR_ERRONEUSNICKNAME 432

// ERR_NICKNAMEINUSE (433): The nickname is already in use by another client.
#define ERR_NICKNAMEINUSE 433

// ERR_USERNOTINCHANNEL (441): The client tried to perform an operation on a user not in the channel.
#define ERR_USERNOTINCHANNEL 441

// ERR_NOTONCHANNEL (442): The client is not in the channel they tried to interact with.
#define ERR_NOTONCHANNEL 442

// ERR_USERONCHANNEL (443): The client tried to invite a user already in the channel.
#define ERR_USERONCHANNEL 443

// ERR_NEEDMOREPARAMS (461): Not enough parameters were provided for a command.
#define ERR_NEEDMOREPARAMS 461

// ERR_ALREADYREGISTERED (462): Sent when a user tries to register more than once (like with the USER command).
#define ERR_ALREADYREGISTERED 462

// ERR_PASSWDMISMATCH (464): Password mismatch error.
#define ERR_PASSWDMISMATCH 464

// ERR_CHANNELISFULL (471): The channel is full and cannot accept more clients.
#define ERR_CHANNELISFULL 471

// ERR_INVITEONLYCHAN (473): The channel is invite-only and the client is not invited.
#define ERR_INVITEONLYCHAN 473

// ERR_BANNEDFROMCHAN (474): The client is banned from the channel.
#define ERR_BANNEDFROMCHAN 474

// ERR_BADCHANNELKEY (475): Incorrect channel key (password).
#define ERR_BADCHANNELKEY 475

// ERR_BADCHANMASK (476): Invalid channel name mask.
#define ERR_BADCHANMASK 476

// ERR_NOPRIVILEGES (481): The client does not have the necessary privileges for an operation.
#define ERR_NOPRIVILEGES 481

// ERR_CHANOPRIVSNEEDED (482): The client is not a channel operator but tried to perform an operator-level operation.
#define ERR_CHANOPRIVSNEEDED 482

// ERR_CANTKILLSERVER (483): The client tried to kill the server, which is not allowed.
#define ERR_CANTKILLSERVER 483

// RPL_TOPIC (332): The current topic of the channel.
#define RPL_TOPIC 332

// RPL_NAMREPLY (353): The list of users in the channel.
#define RPL_NAMREPLY 353

// RPL_ENDOFNAMES (366): Marks the end of the user list.
#define RPL_ENDOFNAMES 366

// ERR_BADPARAM (484) : The parameters are not valid
#define ERR_BADPARAM 484

// Note that the only valid version of the GPL as far as jwSMTP
// is concerned is v2 of the license (ie v2, not v2.2 or v3.x or whatever),
// unless explicitly otherwise stated.
//
// This file is part of the jwSMTP library.
//
//  jwSMTP library is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; version 2 of the License.
//
//  jwSMTP library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with jwSMTP library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// jwSMTP library
//   http://johnwiggins.net
//   smtplib@johnwiggins.net
//
// http://www.boost.org
//#include <boost\thread\thread.hpp>

#include <iostream>
#include "jwsmtp/jwsmtp.h"

using std::cout;
using std::cin;
using std::string;

void Usage() {
   cout << "jwSMTP library demo program\n"
           "demo2 <email toaddress> <email fromaddress> <smtpserver>\n"
           "   e.g.\n"
           "      demo2 recipient@there.com me@server.com mail.server.com\n";
}

int main(int argc, char* argv[])
{
   if(argc != 4) {
      Usage();
      return 0;
   }

   cout << "jwSMTP library demo program\n\n";
   string to(argv[1]);
   string from(argv[2]);
   string smtpserver(argv[3]);

   if(to.length() < 2 || from.length() < 2 || smtpserver.length() < 2) {
      Usage();
      return 0;
   }

   char str[2048];
   cout << "Please enter the subject of the mail\n";
   cin.getline(str, 500);	
   string subject(str);
   strcpy(str, "");

   cout << "Please enter the message body end with \".\" on a line by itself\n";
   string mailmessage;
   while(true) {
      cin.getline(str, 2048);
      if(!strcmp(str, "."))
         break;
		
      mailmessage += str;
      mailmessage += "\r\n";
      strcpy(str, "");
   }

   cout << "\nPlease wait sending mail\n";
   // This is how to tell the mailer class that we are using a direct smtp server
   // preventing the code from doing an MX lookup on 'smtpserver' i.e. passing
   // false as the last parameter.
   jwsmtp::mailer mail(to.c_str(), from.c_str(), subject.c_str(), mailmessage.c_str(),
                       smtpserver.c_str(), jwsmtp::mailer::SMTP_PORT, false);

   // using a local file as opposed to a full path.
   mail.attach("attach.png");

   // add another recipient (carbon copy)
   //mail.addrecipient("someoneelse@somewhere.net", mailer::Cc);

   // set a new smtp server! This is the same as setting a nameserver.
   // this depends on the constructor call. i.e. in the constructor
   // If MXLookup is true this is a nameserver
   // If MXLookup is false this is an smtp server
   //mail.setserver("mail.somewherefun.com");
   // same again except using an IP address instead.
   //mail.setserver("192.168.0.1");

   // boost::thread thrd(mail);
   // thrd.join(); // optional
   // or:-

   // Use authentication
   //mail.username("testuser");
   //mail.password("secret");
   // LOGIN authentication by default
   // if you want plain as opposed to login authentication
   //mail.authtype(jwsmtp::mailer::PLAIN);

   // mail.send();
   mail.operator()();
   cout << mail.response() << "\n";

   //mail.reset(); // now we can mail someone else.
   //mail.setmessage("flibbletooting");
   //mail.setsubject("another message same object");
   //mail.attach("/home/user1/image.gif");
   // or a win example
   //mail.attach("C:\\image.gif");
   //mail.addrecipient("someoneelseagain@foobar.net");

   //mail.operator ()();
   //cout << mail.response() << "\n";
   return 0;
}

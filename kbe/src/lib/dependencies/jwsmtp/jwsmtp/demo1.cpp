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
#include <iostream>
// Please note the jwsmtp library has to be installed
// for this next header to work.
#include "jwsmtp/jwsmtp.h"

// obviously we must escape the quotes i.e. \"
std::string html("<html>"
"<body>"
"This is the html part of the message<br><br>"
"<b>bold</b><br>"
"<i>italic</i><br>"
"<font size=\"7\">Large Text</font><br><br>"
"Or a link: <a href=\"http://johnwiggins.net\">johnwiggins.net</a><br><br>"
"And an image: <br><img alt=\"an image in email\" src=\"http://johnwiggins.net/jwsmtp/example.png\"><br>"
"</body>"
"</html>");

int main(int argc, char* argv[])
{
   // replace the users 'to' and 'from' here before compiling this demo
   jwsmtp::mailer m("***@qq.com", "***@163.com", "subject line",
                    "This is the plain text part of the message", "smtp.163.com",
                    jwsmtp::mailer::SMTP_PORT, false);

   // send a html file (remember you still can send an html file as an attachment
   // but calling this function will allow mime compatible clients to actually
   // display the html if the client is set to show html messages.
   //    m.setmessageHTMLfile("/home/myname/thefile.html");
   
	//经过测试，163支持的auth认证是PLAIN模式  
   m.authtype(jwsmtp::mailer::PLAIN);  

   // Build our html from a string. You can also send html as a vector.
   // i.e.
   //    std::vector<char> htmlvec;
   //    ....add html to the vector.
   //    m.setmessageHTML(htmlvec);
   m.setmessageHTML(html);

   m.username("***@163.com");
   m.password("***");
   m.send(); // send the mail
   std ::cout << m.response() << "\n";
   return 0;
}

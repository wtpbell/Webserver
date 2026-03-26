#!/usr/bin/env perl
use strict;
use warnings;
use CGI;

# Scalar ($)
#  - variables holding single value
#  - for individual elements use $
# Hash (%)
#  - key-value pairs (dictionary)
#  - `my %var` binds the container to the hash

my $cgi = CGI->new();

my $cookie = $cgi->cookie(
    -name  => 'cookie-monster',
    -value => 'NomNomNomNom!'
);

#print $cgi->redirect('https://google.com');

print $cgi->header(
 -status => '200 OK',
 -Server => 'cgi',
 -type => 'text/html',
 -cookie => [$cookie, ],
 );
#print $cgi->header(
#    -cookie => [$cookie, ],
#    -type => 'adf',
#);
#

my $env_entries = join("\n",
map { "<tr><td>$_</td><td>$ENV{$_}</td></tr>" }
sort keys %ENV);

print <<HTML;
<!DOCTYPE html>
<html>
 <head>
  <title>Print env</title>
  <style>
  body {
    max-width: 960px;
  }
  table {
   border-collapse: collapse;
   table-layout: fixed;
   width: 100%;
  }
  th, td {
   border: 1px solid #ccc;
   padding: 8px;
   text-align: left;
   word-wrap: break-word;
  }
  th {
   background: #29476e;
   color: #ffc970;
  }
  tr:nth-child(even) {
   background-color: #f2f2f2;
  }
  tr:nth-child(odd) {
   background-color: #ffffff;
  }
  tr:hover {
   background-color: #ffff99;
  }
  </style>
 </head>
 <body>
  <div>
   <h1>Enviroment Variables</h1>
   <table>
    <tr>
     <th>Key</th>
     <th>Value</th>
    </tr>
    $env_entries
   </table>
  </div>
 </body>
</html>
HTML

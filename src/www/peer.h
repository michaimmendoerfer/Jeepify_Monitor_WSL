const char peer_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Form</title>
<style>
body{ margin: 0;padding: 0;font-family: Arial, Helvetica, sans-serif;background-color: #2c257a;}
.box{ width: 70%%; padding: 10px; position: absolute; top: 50%%; left: 50%%; transform: translate(-50%%,-50%%); background-color: #191919; color: white; text-align: center; border-radius: 24px; box-shadow: 0px 1px 32px 0px rgba(0,227,197,0.59);}
h1{ text-transform: uppercase; font-weight: 500;}
input{ border: 0; background: none; margin: 20px auto; text-align: center; border: 2px solid #4834d4; padding: 14px 10px; width: 40%%; outline: none; border-radius: 24px; color: white; font-size: smaller; transition: 0.3s;}
input:focus{ width: 40%%; border-color:#22a6b3 ;}
input[type='submit']{ border: 0;background: none; margin: 20px auto; text-align: center; border: 2px solid #22a6b3; padding: 14px 10px; width: 120px; outline: none; border-radius: 24px; color: white; transition: 0.3s; cursor: pointer;}
input[type='submit']:hover{ background-color: #22a6b3;}
</style>
</head>
<body>
<p>&nbsp;</p>
<form id="values" class="box" action="/get">
<h1>%PeerName%</h1>
%Periphs%
<div class="part"></div>
<table>

</table>
</form></body></html>
)rawliteral";
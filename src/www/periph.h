const char periph_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>%PeerName% - %PeriphName%</title>
        
        <style>
            body{ margin: 0;padding: 0;font-family: Arial, Helvetica, sans-serif;background-color: #2c257a;}
            .box{ width: 70%%; padding: 10px; position: absolute; top: 50%%; left: 50%%; transform: translate(-50%%,-50%%); background-color: #191919; color: white; text-align: center; border-radius: 12px; box-shadow: 0px 1px 32px 0px rgba(0,227,197,0.59);}
            h1{ text-transform: uppercase; font-weight: 500;}
            input{ border: 0; background: none; margin: 5px auto; text-align: center; border: 2px solid #4834d4; padding: 10px 5px; width: 60%%; outline: none; border-radius: 12px; color: white; font-size: smaller; transition: 0.3s;}
            input:focus{ width: 40%%; border-color:#22a6b3 ;}
            input[type='submit']{ border: 0;background: none; margin: 5px auto; text-align: center; border: 2px solid #22a6b3; padding: 10px 5px; width: 30%%; outline: none; border-radius: 12px; color: white; transition: 0.3s; cursor: pointer;}
            input[type='submit']:hover{ background-color: #22a6b3;}
        </style>
    </head>
    <body>
        <p>&nbsp;</p>
        <div class="box">
            <h1>%PeerName%</h1>
            <h2>%PeriphName%</h2>

            <div class='part'>
                <form id='periph' action='/get'>
                    <input name='PeerName' type='text' placeholder='new PeerName'>

                    <p>&nbsp;</p>

                    <input style='width: 60%%;' name='PeriphName' type='%TYPE%' placeholder='%PeriphName%' />
                    <input style='width: 60%%;' name='Nullwert' type='%TYPE%' placeholder='%Nullwert%' />
                    <input style='width: 60%%;' style='width: 60%%;' name='VperAmp' type='%TYPE%' placeholder='%VperAmp%' />

                    <p>&nbsp;</p>

                    <input name='periph' type='submit' value='back'>
                    <input name='periph' type='submit' value='status'>
                    <input name='periph' type='submit' value='Update'>
                </form>
            </div>
        </div>
    </body>
</html>
)rawliteral";
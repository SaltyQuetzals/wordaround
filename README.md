# Wordaround

Still in development.

Need to get a song from [Online Sequencer](http://www.onlinesequencer.net/)? Try opening a song, pressing Ctrl + Shift/Opt + J, pasting the code below and hitting enter:

    for(var out="",skip=0,i=0;i<song.noteColumns.length;i++){if(skip>0)out+="NOTE_EXT,";else{var note=song.noteColumns[i];if("undefined"==typeof note)out+="0,";else{var count=0,upmostpitch=0,upmostindex;for(var index in note){count++;var pitch=note[index].type,asnum=0;asnum+=24*parseInt(pitch.charCodeAt(pitch.length-1)),asnum+=2*(pitch.charCodeAt(0)-97),"#"==pitch.charCodeAt(1)&&asnum++,asnum>upmostpitch&&(upmostpitch=asnum,upmostindex=index)}note=note[upmostindex],count>1&&console.warn("More than one note at t="+note.intTime+". Only "+note.type+" is used."),skip=note.length,out+="NOTE_"+note.type.replace("#","S")+","}}i%4==3&&i>0&&(out+="\n"),skip--}for(;skip>0;)out+="NOTE_EXT,",skip--;out=out.substr(0,out.length-1);

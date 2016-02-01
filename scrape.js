var out = "", skip = 0;
for (var i = 0; i < song.noteColumns.length; i++) {
    if (skip > 0) {
        out += "NOTE_EXT,"; // more than 1 unit length
    } else {
        var note = song.noteColumns[i];
        if (typeof note == "undefined") {
            out += "0,"; // no tone
        } else {
            var count = 0, upmostpitch = 0, upmostindex;
            for (var index in note) {
                count++;
                // get the highest pitch, convert type to number
                var pitch = note[index].type, asnum = 0;
                asnum += parseInt(pitch.charCodeAt(pitch.length-1))*24;
                asnum += (pitch.charCodeAt(0)-97)*2;
                if (pitch.charCodeAt(1) == "#") {
                    asnum++;
                }
                if (asnum > upmostpitch) { // is this the highest note?
                    upmostpitch = asnum;
                    upmostindex = index;
                }
            }
            note = note[upmostindex];
            if (count > 1) { // warn if more than one note
                console.warn("More than one note at t="+
                    note.intTime+". Only "+note.type+" is used.");
            }
            skip = note.length; // make sure to use extenders if needed
            out += "NOTE_"+note.type.replace("#", "S")+",";
        }
    }
    if (i % 4 == 3 && i > 0) {
        out += "\n"; // break up every four notes
    }
    skip--;
}
while (skip > 0) { // if the last note needs to be extended
    out += "NOTE_EXT,";
    skip--;
}
out = out.substr(0, out.length-1); // remove last comma

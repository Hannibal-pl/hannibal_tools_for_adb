#!/usr/bin/php
<?php
/*
Copyright (c) 2024, Konrad Rzepecki <hannibal@astral.lodz.pl>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OFMERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

define("MODE_NONE", 0);
define("MODE_PARTS", 1);
define("MODE_NET", 2);
define("MODE_POSITIONS", 3);

$mode = MODE_NONE;

$packages = array();
//ripped earlyer as it need to add more data to draw it properly
$packages[] = array('name' => '18 PIN POWER',     'count' => 0, 'pins' =>  18, 'draw' => 'draw_con18');
$packages[] = array('name' => '32 PIN DIAG',      'count' => 0, 'pins' =>  32, 'draw' => 'draw_con32');
$packages[] = array('name' => '50 PIN CONN',      'count' => 0, 'pins' =>  50, 'draw' => 'draw_con50');
$packages[] = array('name' => '96 PIN CONN',      'count' => 0, 'pins' =>  96, 'draw' => 'draw_con96');
$packages[] = array('name' => '100 PIN EDGE',     'count' => 0, 'pins' => 100, 'draw' => 'draw_con100');
$packages[] = array('name' => 'CONN RECPT 9D',    'count' => 0, 'pins' =>  11, 'draw' => 'draw_con9d');
$packages[] = array('name' => 'CONN RECPT 25D',   'count' => 0, 'pins' =>  27, 'draw' => 'draw_con25d');
$packages[] = array('name' => 'SIP10 (062)',      'count' => 0, 'pins' =>  10, 'draw' => 'draw_sip10');
$packages[] = array('name' => '14 DIP',           'count' => 0, 'pins' =>  14, 'draw' => 'draw_dip14');
$packages[] = array('name' => '16 DIP',           'count' => 0, 'pins' =>  16, 'draw' => 'draw_dip16');
$packages[] = array('name' => '18 DIP',           'count' => 0, 'pins' =>  18, 'draw' => 'draw_dip18');
$packages[] = array('name' => '20 DIP',           'count' => 0, 'pins' =>  20, 'draw' => 'draw_dip20');
$packages[] = array('name' => '24 DIP',           'count' => 0, 'pins' =>  24, 'draw' => 'draw_dip24');
$packages[] = array('name' => '28 DIP',           'count' => 0, 'pins' =>  28, 'draw' => 'draw_dip28');
$packages[] = array('name' => '48 DIP',           'count' => 0, 'pins' =>  48, 'draw' => 'draw_dip48');
$packages[] = array('name' => '64 DIP',           'count' => 0, 'pins' =>  64, 'draw' => 'draw_dip64');
$packages[] = array('name' => '24 SDIP',          'count' => 0, 'pins' =>  24, 'draw' => 'draw_sdip24');
$packages[] = array('name' => 'RES (075) 450',    'count' => 0, 'pins' =>   2, 'draw' => 'draw_default');
$packages[] = array('name' => 'RES (075) 500',    'count' => 0, 'pins' =>   2, 'draw' => 'draw_default');
$packages[] = array('name' => 'RES (100) 700',    'count' => 0, 'pins' =>   2, 'draw' => 'draw_default');
$packages[] = array('name' => 'DIODE (075) 600',  'count' => 0, 'pins' =>   2, 'draw' => 'draw_default');
$packages[] = array('name' => 'AXL CAP (062)300', 'count' => 0, 'pins' =>   2, 'draw' => 'draw_default');
$packages[] = array('name' => 'BYPASS CAP',       'count' => 0, 'pins' =>   2, 'draw' => 'draw_default');
$packages[] = array('name' => 'LED',              'count' => 0, 'pins' =>   2, 'draw' => 'draw_default');
$packages[] = array('name' => 'JPAD',             'count' => 0, 'pins' =>   2, 'draw' => 'draw_default');
$packages[] = array('name' => 'TO-18 (075)',      'count' => 0, 'pins' =>   3, 'draw' => 'draw_default');
$packages[] = array('name' => 'CRYSTAL OSC-4',    'count' => 0, 'pins' =>   4, 'draw' => 'draw_default');
$packages[] = array('name' => 'BATT PINS',        'count' => 0, 'pins' =>   3, 'draw' => 'draw_default');
$packages[] = array('name' => 'RSTIFF SHAPE',     'count' => 0, 'pins' =>  13, 'draw' => 'draw_default');


$parts = array();
$nets = array();

if (count($argv) < 2) {
	printf("Usage:\t %s CAD_FILE\n", basename($argv[0]));
	exit(-1);
}

$cadfile = fopen($argv[1], "r");
if ($cadfile == false) {
	printf("Cannot open input file: %s\n", $argv[1]);
	exit(-1);
}

while(!feof($cadfile)) {
	$line = ltrim(fgets($cadfile), "\x00..\x1F");
	switch ($mode) {
		case MODE_NONE:
			parse_none($line);
			break;
		case MODE_PARTS:
			parse_parts($line);
			break;
		case MODE_NET:
			parse_net($line);
			break;
		case MODE_POSITIONS:
			parse_positions($line);
			break;
	}
}

fclose($cadfile);

print("<svg viewBox=\"-2000 0 34000 10000\" width=\"24000\" height=\"10000\" xmlns=\"http://www.w3.org/2000/svg\">\n");
foreach($parts as $p) {
	print($packages[$p['pkg']]['draw']($p));
}
print("</svg>\n");

//print_r($packages);
//print_r($parts);
//print_r($nets);

function parse_none($line) {
	global $mode;

	if (strncmp($line, "PARTS LIST      ", 16) == 0) {
		$mode = MODE_PARTS;
		fprintf(STDERR, "Part List Block Found\n");
		return;
	} else if (strncmp($line, "NET LIST        ", 16) == 0) {
		$mode = MODE_NET;
		fprintf(STDERR, "Net List Block Found\n");
		return;
	} else if (strncmp($line, "POSITION        ", 16) == 0) {
		$mode = MODE_POSITIONS;
		fprintf(STDERR, "Positions Block Found\n");
		return;
	}
}

function parse_parts($line) {
	global $mode;

	if (strncmp($line, "EOS", 3) == 0) {
		$mode = MODE_NONE;
		fprintf(STDERR, "Part List Block End\n");
		return;
	}

	$len = strlen($line);

	if ($len < 43) { //valid line must have at least 43 characters;
		return;
	}

	$title = trim(substr($line, 0, 16));
	$package = trim(substr($line, 16, 17));

	$part[0] = trim(substr($line, 33, 8));
	if ($len > 43) {
		$part[1] = trim(substr($line, 41, 8));
	}
	if ($len > 51) {
		$part[2] = trim(substr($line, 49, 8));
	}
	if ($len > 59) {
		$part[4] = trim(substr($line, 57, 8));
	}
	if ($len > 67) {
		$part[5] = trim(substr($line, 65, 8));
	}

	foreach ($part as $p) {
		add_part($p, $package, $title);
	}

}

function parse_net($line) {
	global $mode;
	static $net;

	if (strncmp($line, "EOS", 3) == 0) {
		$mode = MODE_NONE;
		fprintf(STDERR, "Net List Block End\n");
		return;
	}

	$len = strlen($line);

	if ($len < 9) {
		return;
	}

	if (strncmp($line, "NODENAME", 8) == 0) {
		$net = get_net(trim(substr($line, 9, 17)));
		return;
	} else if (strncmp($line, "NODE", 4) == 0) {
		$net = get_net('NODE_' . trim(substr($line, 6, 4)));
		return;
	}

	//pin list from now
	$pin[0] = array('part' => trim(substr($line, 4, 8)), 'pin' => intval(substr($line, 12, 4)));
	if ($len > 19) {
		$pin[1] = array('part' => trim(substr($line, 16, 8)), 'pin' => intval(substr($line, 24, 4)));
	}
	if ($len > 31) {
		$pin[2] = array('part' => trim(substr($line, 28, 8)), 'pin' => intval(substr($line, 36, 4)));
	}
	if ($len > 43) {
		$pin[3] = array('part' => trim(substr($line, 40, 8)), 'pin' => intval(substr($line, 48, 4)));
	}
	if ($len > 55) {
		$pin[4] = array('part' => trim(substr($line, 52, 8)), 'pin' => intval(substr($line, 60, 4)));
	}

	foreach ($pin as $p) {
		add_pin($net, $p);
	}

}

function parse_positions($line) {
	global $mode;
	global $parts;

	if (strncmp($line, "EOS", 3) == 0) {
		$mode = MODE_NONE;
		fprintf(STDERR, "Positions Block End\n");
		return;
	}

	if (strlen($line) < 26) { //valid line must have at least 26 characters;
		return;
	}

	$part = trim(substr($line, 0, 8));
	$pos_x = fix_number(trim(substr($line, 8, 2) . substr($line, 11, 3)));
	$pos_y = fix_number(trim(substr($line, 16, 2) . substr($line, 19, 3)));

	$p = get_part($part);
	$parts[$p]['x'] = $pos_x;
	$parts[$p]['y'] = $pos_y - 12600;

}

function add_part($part, $package, $title) {
	global $parts;

	$id = array_push($parts, array('part' => $part, 'title' => $title, 'pkg' => get_package($package), 'pins' => array())) - 1;
	$parts[$id]['id'] = $id;
}


function get_package($pkg) {
	global $packages;
	foreach ($packages as $i => $package) {
		if ($package['name'] == $pkg) {
			$packages[$i]['count']++;
			return $i;
		}
	}

	return array_push($packages, array('name' => $pkg, 'count' => 1, 'pins' => 0, 'draw' => 'draw_default')) - 1;
}

function get_part($part) {
	global $parts;
	foreach ($parts as $i => $p) {
		if ($p['part'] == $part) {
			return $i;
		}
	}
	return false;
}

function get_net($net) {
	global $nets;
	foreach ($nets as $i => $n) {
		if ($n['name'] == $net) {
			return $i;
		}
	}

	return array_push($nets, array('name' => $net, 'parts' => array())) - 1;
}

function add_pin($net, $pin) {
	global $parts;
	global $nets;

	$part = get_part($pin['part']);

	array_push($nets[$net]['parts'], array('part' => $part, 'pin' => $pin['pin']));
	$parts[$part]['pins'][$pin['pin']] = $net;
}

function fix_number($number) {
	$count = 0;
	$number = str_replace(array("/", ".", "-", ",", "+", "*", ")". "(", "'"), array("1", "2", "3", "4", "5", "6", "7", "8", "9"), $number, $count);
	if ($count > 0) {
		return -intval($number);
	} else {
		return intval($number);
	}
}

//drawing functions

function draw_default($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 30 v -30 h -30 v 30 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 20)."\" y=\"".($data['y'] + 20)."\" style=\"font-size: 20pt; fill: green;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_con18($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 60 v -540 h -60 v 540 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 20)."\" y=\"".($data['y'] + 20)."\" style=\"font-size: 30pt; fill: blue;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_con32($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 60 v -960 h -60 v 960 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 20)."\" y=\"".($data['y'] + 20)."\" style=\"font-size: 30pt; fill: blue;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_con50($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 60 v -1500 h -60 v 1500 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 20)."\" y=\"".($data['y'] + 20)."\" style=\"font-size: 30pt; fill: blue;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_con96($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 60 v -2880 h -60 v 2880 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 20)."\" y=\"".($data['y'] + 20)."\" style=\"font-size: 30pt; fill: blue;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_con100($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 60 v -3000 h -60 v 3000 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 20)."\" y=\"".($data['y'] + 20)."\" style=\"font-size: 30pt; fill: blue;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_con9d($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 60 v -150 h -60 v 150 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 20)."\" y=\"".($data['y'] + 20)."\" style=\"font-size: 30pt; fill: blue;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_con25d($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 60 v -390 h -60 v 390 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 20)."\" y=\"".($data['y'] + 20)."\" style=\"font-size: 30pt; fill: blue;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_sip10($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 30 v -300 h -30 v 300 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 20)."\" y=\"".($data['y'] + 20)."\" style=\"font-size: 30pt; fill: blue;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_dip14($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 90 v -210 h -90 v 210 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 30)."\" y=\"".($data['y'] + 30)."\" style=\"font-size: 30pt; fill: red;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_dip16($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 90 v -240 h -90 v 240 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 30)."\" y=\"".($data['y'] + 30)."\" style=\"font-size: 30pt; fill: red;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_dip18($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 90 v -270 h -90 v 270 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 30)."\" y=\"".($data['y'] + 30)."\" style=\"font-size: 30pt; fill: red;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_dip20($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 90 v -300 h -90 v 300 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 30)."\" y=\"".($data['y'] + 30)."\" style=\"font-size: 30pt; fill: red;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_dip24($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 90 v -360 h -90 v 360 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 30)."\" y=\"".($data['y'] + 30)."\" style=\"font-size: 30pt; fill: red;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_dip28($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 90 v -420 h -90 v 420 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 30)."\" y=\"".($data['y'] + 30)."\" style=\"font-size: 30pt; fill: red;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_dip48($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 180 v -720 h -180 v 720 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 30)."\" y=\"".($data['y'] + 30)."\" style=\"font-size: 30pt; fill: red;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_dip64($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 180 v -960 h -180 v 960 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 30)."\" y=\"".($data['y'] + 30)."\" style=\"font-size: 30pt; fill: red;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}

function draw_sdip24($data) {
	$ret  = "\t<g class=\"part_".$data['id']."\">\n";
	$ret .= "\t\t<path d=\"M ".$data['x']." ".$data['y']." h 90 v -252 h -90 v 252 \" />\n";
	$ret .= "\t\t<text x=\"".($data['x'] + 30)."\" y=\"".($data['y'] + 30)."\" style=\"font-size: 30pt; fill: red;\">".$data['part']." (".$data['title'].")</text>\n";
	$ret .= "\t</g>\n";
	return $ret;
}


?>

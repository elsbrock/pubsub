$INFO = 0x0f;
$WARN = 0x07;
$ERR  = 0x03;
$DEBUG= 0x01;

$curr_level = $INFO;

sub vlog {
    my ($level, $msg) = @_;

    $strlevl = "INVALID";
    if (0) {
    } elsif (($level & $INFO) == $curr_level) {
        $strlevl = "INFO";
    } elsif (($level & $WARN) == $curr_level) {
        $strlevl = "WARN";
    } elsif (($level & $ERR ) == $curr_level) {
        $strlevl = "ERROR";
    } elsif (($level & $DEBUG) == $curr_level) {
        $strlevl = "DEBUG";
    }

    if (($level & $curr_level) == $curr_level) {
        printf("%s - %s\n", $strlevl, $msg);
    } else {
        printf("%s - SKIPPED LEVEL %s\n", $strlevl, $level);
    }
}

$curr_level = $INFO;
printf("current level: INFO\n");
vlog($INFO, "infomsg");
vlog($WARN, "warnmsg");
vlog($ERR, "errmsg");
vlog($DEBUG, "debugmsg");
print "\n";

$curr_level = $WARN;
printf("current level: WARN\n");
vlog($INFO, "infomsg");
vlog($WARN, "warnmsg");
vlog($ERR, "errmsg");
vlog($DEBUG, "debugmsg");
print "\n";

$curr_level = $ERR;
printf("current level: ERR\n");
vlog($INFO, "infomsg");
vlog($WARN, "warnmsg");
vlog($ERR, "errmsg");
vlog($DEBUG, "debugmsg");
print "\n";

$curr_level = $DEBUG;
printf("current level: DEBUG\n");
vlog($INFO, "infomsg");
vlog($WARN, "warnmsg");
vlog($ERR, "errmsg");
vlog($DEBUG, "debugmsg");
print "\n";

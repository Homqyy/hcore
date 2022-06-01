#!/usr/bin/perl
#

use Getopt::Long;

############################################# Function

use constant {
    KEY    => 0,
    VALUE  => 1,
};

use constant {
    MAJOR_VER   => 0,
    MINOR_VER   => 1,
    PATCH_VER   => 2,
};

sub init_vars
{
    my $gcmake = shift;
    my $config_path = $gcmake->{args}->{config_path};

    my $version_file = "$config_path/.version";
    my $next_ver = "1.1.0";
    my %vars = (
        '$VERSION' => "1.0.0",
    );

    if (-e $version_file)
    {
        open my $version_fd, "<", $version_file
            or die "don't open file $version_file: $!";

        local $/;
        my @ver;
        while (<$version_fd>)
        {
            @ver = (/^(\d++).(\d++).(\d++)\n?$/s);

            die "invalid version in $version_file" unless @ver;

            printf "# current version: %s, %s, %s\n",
                $ver[MAJOR_VER], $ver[MINOR_VER], $ver[PATCH_VER];

        }

        close $version_fd;

        $vars{'$VERSION'} = "$ver[MAJOR_VER].$ver[MINOR_VER].$ver[PATCH_VER]";

        $ver[MINOR_VER] += 1;

        $next_ver = "$ver[MAJOR_VER].$ver[MINOR_VER].$ver[PATCH_VER]"
    }

    $gcmake->{next_version} = $next_ver;

    return \%vars;
}

sub next_version
{
    my $gcmake = shift;
    my $config_path = $gcmake->{args}->{config_path};

    my $version_file = "$config_path/.version";

    print "# next version: $gcmake->{next_version}\n";

    open my $version_fd, "+>", $version_file
        or die "don't open file $version_file: $!";

    print {$version_fd} $gcmake->{next_version};

    close $version_fd;
}

sub parse_config
{
    $gcmake = shift;
    my $config_path = $gcmake->{args}->{config_path};

    my $config_file = "$config_path/config";

    my $conf = {};

    my $vars = &init_vars($gcmake, $config_path);
    gcmake->{vars} = $vars;

    open my $config_fd, "<", $config_file
        or die "don't open file $config_file: $!";

    my $line_n = 0;
    while (<$config_fd>)
    {
        $line_n++;

        chomp;

        next if /^\s*#/;
        next if /^$/;

        my @cl = (/^([^=]++)=([^ ]++)$/);

        printf "# parse: %s, %s\n", $cl[KEY], $cl[VALUE];

        if ($cl[VALUE] =~ '^\$')
        {
            return undef, "invalid variable '$cl[VALUE]' in $config_file: $line_n"
                unless $vars->{$cl[VALUE]};
            
            $cl[VALUE] = $vars->{$cl[VALUE]};
        }

        $conf->{$cl[KEY]} = $cl[VALUE];

        print "# insert: $cl[KEY] => $cl[VALUE]\n";
    }

    close $config_fd;

    print "# parse done\n";
    return $conf, undef;
}

sub get_options
{
    my %args;

    my $config_path   = ".gcmake";
    my $next_minor    = '';
    my $next_major    = '';

    GetOptions (
        "config_path=s" => \$config_path,
        "next_major" => \$next_major,
        "next_minor" => \$next_minor,
    );

    $args{config_path} = $config_path;
    $args{next_minor} = $next_minor;
    $args{next_major} = $next_major;

    return \%args;
}

############################################# Main

my %gcmake;

$gcmake{args} = &get_options;

my ($profile, $cmakelists) = @ARGV;

my $conf, $err;

if (-d $gcmake{args}->{config_path})
{
    ($conf, $err) = &parse_config(\%gcmake);

    if ($err)
    {
        print "$err\n";
        exit 1;
    }
}

open my $profile_fd, "<", $profile
    or die "don't open file $profile: $!";

open my $cmakelists_fd, "+>", $cmakelists
    or die "don't open file $cmakelists: $!";

while (<$profile_fd>)
{
    foreach my $key (keys %{$conf})
    {
        s/@@\{$key\}@@/$conf->{$key}/g;
    }

    print {$cmakelists_fd} $_;
}

&next_version($gcmake) if $gcmake{args}->{next_minor};

close $profile_fd;
close $cmakelists_fd;

__END__

=encoding utf8

=head1 NAME

gen_cmakelists.pl - Generate CMakeLists Tool

=head1 SYNOPSIS

gen_cmakelists.pl [I<OPTIONS>] [--] <I<profile>> <I<cmakelists>>

 OPTIONS:
    --config_path <config_path>        set a path of config
    --next_minor                       auto increase minor

=head1 DESCRIPTION

=head1 CONSTRUCTOR ARGUMENTS

=head1 EXAMPLES

C< ./tools/gen_cmakelists.pl --next_minor --  CMakeLists.txt.in CMakeLists.txt >

=head1 SEE ALSO

=head1 AUTHOR

=over 4

=item * Name: 王鸿奇 (Homqyy)

=item * Email: yilupiaoxuewhq@163.com

=item * Blog: https://www.homqyy.cn

=back

=head1 COPYRIGHT

Homqyy

=cut
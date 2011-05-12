package Zoo::Model::DB;

use strict;
use base 'Catalyst::Model::DBIC::Schema';

__PACKAGE__->config(
    schema_class => 'Zoo::Schema',
    
    connect_info => {
        dsn => 'dbi:SQLite:zoo.db',
        user => '',
        password => '',
        on_connect_do => q{PRAGMA foreign_keys = ON},
    }
);

=head1 NAME

Zoo::Model::DB - Catalyst DBIC Schema Model

=head1 SYNOPSIS

See L<Zoo>

=head1 DESCRIPTION

L<Catalyst::Model::DBIC::Schema> Model using schema L<Zoo::Schema>

=head1 METHODS

=cut


=head2 twig_nest

Returns an XML::Twig as a tree of nested anonymous arrays of tag=>value pairs.

=cut

sub twig_nest {
    my ($self, $elt) = @_;
    if (ref($elt) eq 'XML::Twig') {
	$elt = $elt->root;
    }
    my @child = $elt->children;
    return @child > 0
	? [map (($_->tag => $self->twig_nest($_)), @child)]
	: $_->text;
}

=head2 particle_names

Returns a list of typenames of particles in a given XML::Twig tree.

=cut

sub particle_names {
    my ($self, $twig) = @_;
    my %particle_hash = map (($_->text => 1),
			     $twig->descendants('type'));
    return sort keys %particle_hash;
}

=head2 worlds

=cut

sub worlds {
    my ($self) = @_;
    my $world = $self->resultset('World')->all;
    return $world;
}


=head2 world_by_id

=cut

sub world_by_id {
    my ($self, $worldId) = @_;
#    $self->storage->debug(1);
    my $world = $self->resultset('World')->find($worldId);
    return $world;
}

=head2 particles_by_name

=cut

sub particles_by_name {
    my ($self, @particle_names) = @_;
    my @particles = $self->resultset('Particle')->search([map ({ 'name' => $_ }, @particle_names)]);
    return @particles;
}

=head2 descendant_particles

=cut

sub descendant_particles {
    my ($self, $twig) = @_;
    my @particle_names = $self->particle_names ($twig);
    my @particles = $self->particles_by_name (@particle_names);
    my @descendants = map ($_->descendants, @particles);
    my %descendant_hash = map (($_->name => $_), @particles, @descendants);
    return values %descendant_hash;
}

=head1 GENERATED BY

Catalyst::Helper::Model::DBIC::Schema - 0.48

=head1 AUTHOR

Ian Holmes

=head1 LICENSE

This library is free software, you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut

1;

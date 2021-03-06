package Twiggy;
use Moose;
use Twiggy::Elt;
use namespace::autoclean;

extends 'XML::Twig';


=head1 NAME

Twiggy - XML::Twig derived class

=head1 SYNOPSIS

XML::Twig, with minor enhancements

=head1 DESCRIPTION

L<XML::Twig> derived class, offering a few helper methods for the Zoo.

=head1 METHODS

=cut

=head2 root

Casts super method to L<Twiggy::Elt>

=cut

sub root {
    my ($self) = @_;
    my $root = $self->SUPER::root();
    bless $root, 'Twiggy::Elt';
    return $root;
}

=head2 twig_nest

Converts an L<XML::Twig> into a tree of nested anonymous arrays of tag=>value pairs.

Wrapper for recursive Twiggy::Elt method.

=cut

sub twig_nest {
    my ($self, $elt) = @_;
    if (!defined ($elt)) {
	$elt = $self->root;
    }
    return $elt->twig_nest;
}

=head2 particle_names

Returns a list of particles named by a given L<XML::Twig> tree.

More specifically: fetches the content of all the <type>, <gtype> & <gstate> elements,
and all the <bmatch type="..."> attributes.

=cut

sub particle_names {
    my ($self) = @_;
    my %particle_hash = ( map (($_->text => 1),
			       $self->descendants('type'),
			       $self->descendants('gtype'),
			       $self->descendants('gstate')),
			  map (($_->{'att'}->{'type'} => 1),
			       $self->descendants('bmatch')) );
    return sort keys %particle_hash;
}

=head1 AUTHOR

Ian Holmes

=head1 LICENSE

This library is free software, you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut

1;

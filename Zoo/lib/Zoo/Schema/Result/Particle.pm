package Zoo::Schema::Result::Particle;

# Created by DBIx::Class::Schema::Loader
# DO NOT MODIFY THE FIRST PART OF THIS FILE

use strict;
use warnings;

use Moose;
use MooseX::NonMoose;
use namespace::autoclean;
extends 'DBIx::Class::Core';

__PACKAGE__->load_components("InflateColumn::DateTime");

=head1 NAME

Zoo::Schema::Result::Particle

=cut

__PACKAGE__->table("particle");

=head1 ACCESSORS

=head2 name

  data_type: 'varchar'
  is_nullable: 0
  size: 255

=head2 image_name

  data_type: 'varchar'
  is_foreign_key: 1
  is_nullable: 1
  size: 255

=head2 cost

  data_type: 'decimal'
  is_nullable: 1

=head2 xml

  data_type: 'text'
  is_nullable: 1

=cut

__PACKAGE__->add_columns(
  "name",
  { data_type => "varchar", is_nullable => 0, size => 255 },
  "image_name",
  { data_type => "varchar", is_foreign_key => 1, is_nullable => 1, size => 255 },
  "cost",
  { data_type => "decimal", is_nullable => 1 },
  "xml",
  { data_type => "text", is_nullable => 1 },
);
__PACKAGE__->set_primary_key("name");

=head1 RELATIONS

=head2 image_name

Type: belongs_to

Related object: L<Zoo::Schema::Result::Image>

=cut

__PACKAGE__->belongs_to(
  "image_name",
  "Zoo::Schema::Result::Image",
  { name => "image_name" },
  {
    is_deferrable => 1,
    join_type     => "LEFT",
    on_delete     => "CASCADE",
    on_update     => "CASCADE",
  },
);

=head2 particle_dep_dep_names

Type: has_many

Related object: L<Zoo::Schema::Result::ParticleDep>

=cut

__PACKAGE__->has_many(
  "particle_dep_dep_names",
  "Zoo::Schema::Result::ParticleDep",
  { "foreign.dep_name" => "self.name" },
  { cascade_copy => 0, cascade_delete => 0 },
);

=head2 particle_dep_particle_names

Type: has_many

Related object: L<Zoo::Schema::Result::ParticleDep>

=cut

__PACKAGE__->has_many(
  "particle_dep_particle_names",
  "Zoo::Schema::Result::ParticleDep",
  { "foreign.particle_name" => "self.name" },
  { cascade_copy => 0, cascade_delete => 0 },
);

=head2 inventories

Type: has_many

Related object: L<Zoo::Schema::Result::Inventory>

=cut

__PACKAGE__->has_many(
  "inventories",
  "Zoo::Schema::Result::Inventory",
  { "foreign.particle_name" => "self.name" },
  { cascade_copy => 0, cascade_delete => 0 },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-05-11 12:57:12
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:YdhS2zICS2hUmTyipuruzw


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
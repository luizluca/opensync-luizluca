OSyncMappingTable *osync_mappingtable_new(OSyncGroup *);
void osync_mappingtable_load(OSyncMappingTable *table);
void osync_mappingtable_close(OSyncMappingTable *table);
void osync_mappingtable_save_change(OSyncMappingTable *table, OSyncChange *change);
void osync_mappingtable_delete_change(OSyncMappingTable *table, OSyncChange *change);

OSyncMapping *osync_mappingtable_nth_mapping(OSyncMappingTable *table, int num);
int osync_mappingtable_num_mappings(OSyncMappingTable *table);
OSyncMapping *osync_mappingtable_get_mapping(OSyncMappingTable *table, OSyncMember *member, char *uid);
OSyncMapping *osync_mapping_new(OSyncMappingTable *table);
int osync_mappingtable_num_rows(OSyncMappingTable *table);
OSyncMapping *osync_mappingtable_nth_row(OSyncMappingTable *table, int num);
int osync_mapping_num_entries(OSyncMapping *mapping);
OSyncChange *osync_mapping_nth_entry(OSyncMapping *mapping, int nth);
//OSyncMember *osync_mappingentry_get_member(OSyncChange *entry);
//OSyncChange *osync_mappingentry_get_change(OSyncChange *entry);
//char *osync_mappingentry_get_uid(OSyncChange *entry);
void osync_mapping_update_change(OSyncMapping *mapping, OSyncMember *member, OSyncChange *change);
//void osync_mapping_update_uid(OSyncMapping *mapping, OSyncMember *member, char *uid);
//OSyncChange *osync_mappingentry_new(OSyncMember *member);
//OSyncMapping *osync_mappingentry_get_mapping(OSyncChange *entry);
//void osync_mappingentry_set_state(OSyncChange *entry, OSyncChangeState state);
//void osync_mappingentry_copy_change(OSyncChange *source, OSyncChange *target);
OSyncChange *osync_mapping_get_entry_by_owner(OSyncMapping *mapping, OSyncMember *member);
void *osync_mapping_get_engine_data(OSyncMapping *mapping);
void osync_mapping_set_engine_data(OSyncMapping *mapping, void *engine_data);
//unsigned long *osync_mappingentry_get_flags(OSyncChange *entry);
void osync_mapping_set_masterentry(OSyncMapping *mapping, OSyncChange *master);
OSyncChange *osync_mapping_get_masterentry(OSyncMapping *mapping);
long long int osync_mapping_get_id(OSyncMapping *mapping);
void osync_mapping_set_id(OSyncMapping *mapping, long long int id);
//void osync_mapping_create(OSyncMappingTable *table, OSyncMapping *mapping);
OSyncGroup *osync_mapping_get_group(OSyncMapping *mapping);
void osync_mappingtable_set_dbpath(OSyncMappingTable *table, const char *path);
void osync_mapping_delete(OSyncMapping *mapping);
int osync_mappingtable_num_unmapped(OSyncMappingTable *table);
OSyncChange *osync_mappingtable_nth_unmapped(OSyncMappingTable *table, int i);
void osync_mappingtable_add_unmapped(OSyncMappingTable *table, OSyncChange *change);
void osync_mappingtable_remove_unmapped(OSyncMappingTable *table, OSyncChange *change);
void osync_mapping_remove_entry(OSyncMapping *mapping, OSyncChange *entry);
void osync_mappingtable_free(OSyncMappingTable *table);
void osync_mapping_free(OSyncMapping *mapping);
void osync_mapping_add_entry(OSyncMapping *mapping, OSyncChange *entry);
void osync_mappingtable_set_slow_sync(OSyncMappingTable *table, const char *objtype);

/*-
 * Copyright (C) 2011  Oy L M Ericsson Ab, NomadicLab
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of the
 * BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

/* Simple text subscriber example. */

public class test3 {
    static {
      System.loadLibrary("blackadder_java");
    }
    static final blackadder_java.BA BA = null;

    public static void main(String args[]) throws Exception {
        blackadder_java.Blackadder ba =
	    blackadder_java.Blackadder.Instance(true);

	byte[] prefix_id = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	byte[] id = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

	byte[] full_id = new byte[prefix_id.length + id.length];
	System.arraycopy(prefix_id, 0, full_id, 0, prefix_id.length);
        System.arraycopy(id, 0, full_id, prefix_id.length, id.length);
	
	//	ba.publish_scope(prefix_id, new byte[0], BA.NODE_LOCAL, null);
	ba.subscribe_scope(id,prefix_id,BA.NODE_LOCAL, null);	
	
	byte[] data = { '1', '2', '3', 'A', 'B' };

	byte[] sid = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
        byte[] rid = { 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12 };

	
	byte[] target_id = new byte[sid.length +rid.length];
	System.arraycopy(sid, 0, target_id, 0, sid.length);
        System.arraycopy(rid, 0, target_id, sid.length, rid.length);

	ba.publish_data(target_id, BA.NODE_LOCAL, null, data);

        while (true) {
            blackadder_java.Event ev = new blackadder_java.Event();
            ba.getEvent(ev);
            int type = ev.getType();
            System.out.println(type);
            if (type == 0)
                break;
            if (type == BA.PUBLISHED_DATA) {
                byte[] data2 = ev.getData();
                System.out.println(ev.getData_len());
                System.out.println(new String(data2));
            }
        }

        ba.disconnect();
    }
}

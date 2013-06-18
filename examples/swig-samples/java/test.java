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

/* Simple Blackadder connection example. */

public class test {
    static {
        System.loadLibrary("blackadder_java");
    }
    static final blackadder_java.BA BA = null;

    void subscribe2(byte[] sid, byte[] rid) {
        blackadder_java.Blackadder ba =
	    blackadder_java.Blackadder.Instance(true);

        ba.subscribe_scope(rid, sid, BA.DOMAIN_LOCAL, null);


        while (true) {
            blackadder_java.Event ev = new blackadder_java.Event();
            ba.getEvent(ev);
            int type = ev.getType();
            System.out.println(type);
            if (type == 0)
                break;
            if (type == BA.PUBLISHED_DATA) {
                byte[] data = ev.getData();
                System.out.println(ev.getData_len());
                System.out.println(new String(data));
            }
        }

        ba.disconnect();
    }

    public static void main(String args[]) {
        blackadder_java.Blackadder ba =
	    blackadder_java.Blackadder.Instance(true);
        ba.publish_scope("00001111".getBytes(), new byte[0],
                         BA.DOMAIN_LOCAL, null);
	System.out.println("XXXXYYYY".getBytes());
	System.out.println("00001111".getBytes());
	// ba.publish_info("XXXXYYYY".getBytes(), "00001111".getBytes(),
        //                BA.DOMAIN_LOCAL, null);

	byte[] prefix_id = { 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b };
	byte[] id = { 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d };

	byte[] full_id = new byte[prefix_id.length + id.length];
	System.arraycopy(prefix_id, 0, full_id, 0, prefix_id.length);
        System.arraycopy(id, 0, full_id, prefix_id.length, id.length);
	
	ba.publish_scope(prefix_id, new byte[0], BA.DOMAIN_LOCAL, null);
	
	
	ba.publish_info(id, prefix_id, BA.DOMAIN_LOCAL, null);
	byte[] data = { '1', '2', '3', 'A', 'B', 'C' };


       
	while (true) {
            blackadder_java.Event ev = new blackadder_java.Event();
            ba.getEvent(ev);
            int type = ev.getType();
	    if (type == 0)
                break;
	    if (type == BA.START_PUBLISH){
	      	ba.publish_data(full_id, BA.DOMAIN_LOCAL, null, data);
	    }
        }

        ba.disconnect();
    }
}

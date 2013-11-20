/*
 *  Collin's Dynamic Dalvik Instrumentation Toolkit for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
 *
 *  License: LGPL v2.1
 *
 */

package org.mulliner.ddiexample;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Calendar;
import java.util.GregorianCalendar;

import android.app.Application;
import android.content.Intent;
import android.telephony.PhoneNumberUtils;
import android.telephony.SmsMessage;

// takes incoming SMS message, reverses the body message and injects it back into the system (will appear as a 2nd SMS message)
public class SMSDispatch 
{	
	public SMSDispatch(byte pdus[][])
	{
		System.out.println("org.mulliner.ddiexample.SMSDispatch(pdu)");
		
		SmsMessage s1 = SmsMessage.createFromPdu(pdus[0]);
		if (s1 != null) {
			System.out.println("ddiexample: incoming SMS");
			System.out.println("ddiexample: " + s1.getMessageBody());
			System.out.println("ddiexample: " + s1.getOriginatingAddress());
		
			if (s1.getMessageBody() != null) {
		
				String rs = "";
				// reverse message
				for (int i = s1.getMessageBody().length() - 1; i >= 0; i--)
					rs = rs + s1.getMessageBody().charAt(i);
				
				byte p[] = createFakeSms(s1.getOriginatingAddress(), rs);
				byte sp[][] = new byte[1][p.length];
				System.arraycopy(p, 0, sp[0], 0, p.length);
				
				System.out.println("ddiexample: fake SMS");
				System.out.println("ddiexample: " + rs);
				
				Intent intent = new Intent("android.provider.Telephony.SMS_RECEIVED");
				intent.putExtra("pdus", sp);
				intent.putExtra("format", "3gpp");
				
				System.out.println(intent.toString());
		
				// get a context
				Application a = getcon();
				// send intent
				a.sendBroadcast(intent, "android.permission.RECEIVE_SMS");
				System.out.println("ddiexample: appname: " + a.toString());
			}
		}
	}
	
	public Application getcon() 
	{
		try {
		    final Class<?> activityThreadClass =
		            Class.forName("android.app.ActivityThread");
		    if (activityThreadClass == null)
		    	System.out.println("activityThreadClass == null");
		    final Method method = activityThreadClass.getMethod("currentApplication");
		    Application app = (Application) method.invoke(null, (Object[]) null);
		    if (app == null) {
		    	System.out.println("getcon app == null");
		    	final Method method2 = activityThreadClass.getMethod("getApplication");
		    	if (method2 == null)
		    		System.out.println("method2 == null");
		    	if (app == null) {
		    		System.out.println("getcon 2 app == null");
		    	try {
		    	Field f = activityThreadClass.getField("mInitialApplication");
		    	app = (Application) f.get(activityThreadClass);
		    	} catch (Exception e) {}
		    	}
		    	if (app == null)
		    		System.out.println("getcon 3 app == null");
		    }
		    return app;
		} catch (final ClassNotFoundException e) {
		    // handle exception
			System.out.println(e.toString());
		} catch (final NoSuchMethodException e) {
		    // handle exception
			System.out.println(e.toString());
		} catch (final IllegalArgumentException e) {
		    // handle exception
			System.out.println(e.toString());
		} catch (final IllegalAccessException e) {
		    // handle exception
			System.out.println(e.toString());
		} catch (final InvocationTargetException e) {
		    // handle exception
			System.out.println(e.toString());
		}
		System.out.println("getcon == null :-(");
		return null;
	}
	
	//
	// reverseBytes and createFakesSms code taken from Thomas Cannon's SMSSpoof.java
	// https://github.com/thomascannon/android-sms-spoof/blob/master/SMSSpoofer/src/net/thomascannon/smsspoofer/SMSSpoof.java
	//
	private static byte reverseByte(byte b) 
	{
        return (byte) ((b & 0xF0) >> 4 | (b & 0x0F) << 4);
    }
	
	private static byte[] createFakeSms(String sender, String body)
	{
		//Source: http://stackoverflow.com/a/12338541
		//Source: http://blog.dev001.net/post/14085892020/android-generate-incoming-sms-from-within-your-app
        byte[] scBytes = PhoneNumberUtils.networkPortionToCalledPartyBCD("0000000000");
        byte[] senderBytes = PhoneNumberUtils.networkPortionToCalledPartyBCD(sender);
        int lsmcs = scBytes.length;
        byte[] dateBytes = new byte[7];
        Calendar calendar = new GregorianCalendar();
        dateBytes[0] = reverseByte((byte) (calendar.get(Calendar.YEAR)));
        dateBytes[1] = reverseByte((byte) (calendar.get(Calendar.MONTH) + 1));
        dateBytes[2] = reverseByte((byte) (calendar.get(Calendar.DAY_OF_MONTH)));
        dateBytes[3] = reverseByte((byte) (calendar.get(Calendar.HOUR_OF_DAY)));
        dateBytes[4] = reverseByte((byte) (calendar.get(Calendar.MINUTE)));
        dateBytes[5] = reverseByte((byte) (calendar.get(Calendar.SECOND)));
        dateBytes[6] = reverseByte((byte) ((calendar.get(Calendar.ZONE_OFFSET) + calendar
                .get(Calendar.DST_OFFSET)) / (60 * 1000 * 15)));
        try {
            ByteArrayOutputStream bo = new ByteArrayOutputStream();
            bo.write(lsmcs);
            bo.write(scBytes);
            bo.write(0x04);
            bo.write((byte) sender.length());
            bo.write(senderBytes);
            bo.write(0x00);
            bo.write(0x00); // encoding: 0 for default 7bit
            bo.write(dateBytes);
            try {
                String sReflectedClassName = "com.android.internal.telephony.GsmAlphabet";
                Class cReflectedNFCExtras = Class.forName(sReflectedClassName);
                Method stringToGsm7BitPacked = cReflectedNFCExtras.getMethod("stringToGsm7BitPacked", new Class[] { String.class });
                stringToGsm7BitPacked.setAccessible(true);
                byte[] bodybytes = (byte[]) stringToGsm7BitPacked.invoke(null, body);
                bo.write(bodybytes);
            } catch (Exception e) {
            }

            return bo.toByteArray();
        } catch (IOException e) {}
       return null;
	}
}

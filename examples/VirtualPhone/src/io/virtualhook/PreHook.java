package io.virtualhook;

import java.lang.reflect.Method;

import android.content.ContentResolver;
import android.content.Context;
import android.net.wifi.WifiInfo;
import android.provider.Settings.Secure;
import android.telephony.TelephonyManager;
import apk.andhook.AndHook;

public class PreHook {
	private static final String IMEI = "860102017052017";
	private static final String IMSI = "460072017147709";
	private static final String MSISDN = "18802012886";
	private static final String ICCID = "89860032017083852017";
	private static final String ANDROID_ID = "20175dbee83xyapp";
	private static final String MAC = "08:08:7C:88:A6:88";

	public String getDeviceId() {
		LogUtil.i("hit");
		return IMEI;
	}

	public String getDeviceId(int slot) {
		LogUtil.i("hit slot = " + slot);
		return IMEI;
	}

	private static void fake_IMEI() {
		final String target = "getDeviceId";
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target, (Class<?>[]) null);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					(Class<?>[]) null);
			AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
			LogUtil.e(target, e);
		}

		// android 5.x or later
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target, int.class);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					int.class);
			if (origin != null)
				AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
		}
	}

	public String getSubscriberId() {
		LogUtil.i("hit");
		return IMSI;
	}

	public String getSubscriberId(long slot) {
		LogUtil.i("hit slot = " + slot);
		return IMSI;
	}

	private static void fake_IMSI() {
		final String target = "getSubscriberId";
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target, (Class<?>[]) null);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					(Class<?>[]) null);
			AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
			LogUtil.e(target, e);
		}

		// android 5.x or later
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target, long.class);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					long.class);
			if (origin != null)
				AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
		}
	}

	public String getSimSerialNumber() {
		LogUtil.i("hit");
		return ICCID;
	}

	public String getSimSerialNumber(long subId) {
		LogUtil.i("hit subId = " + subId);
		return ICCID;
	}

	private static void fake_ICCID() {
		final String target = "getSimSerialNumber";
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target, (Class<?>[]) null);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					(Class<?>[]) null);
			AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
			LogUtil.e(target, e);
		}

		// android 5.x or later
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target, long.class);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					long.class);
			if (origin != null)
				AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
		}
	}

	public String getLine1Number() {
		LogUtil.i("hit");
		return MSISDN;
	}

	public String getLine1NumberForSubscriber(long subId) {
		LogUtil.i("hit subId = " + subId);
		return MSISDN;
	}

	private static void fake_MSISDN() {
		final String target = "getLine1Number";
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target, (Class<?>[]) null);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					(Class<?>[]) null);
			AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
			LogUtil.e(target, e);
		}

		// android 5.x or later
		final String target2 = "getLine1NumberForSubscriber";
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target2, long.class);
			final Method fake = PreHook.class.getDeclaredMethod(target2,
					long.class);
			if (origin != null)
				AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
		}
	}

	public int getSimState() {
		LogUtil.i("hit");
		return TelephonyManager.SIM_STATE_READY;
	}

	public int getSimState(int slotId) {
		LogUtil.i("hit subId = " + slotId);
		return TelephonyManager.SIM_STATE_READY;
	}

	private static void fake_SIM() {
		final String target = "getSimState";
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target, (Class<?>[]) null);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					(Class<?>[]) null);
			AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
			LogUtil.e(target, e);
		}

		// android 5.x or later
		try {
			final Method origin = TelephonyManager.class.getDeclaredMethod(
					target, int.class);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					int.class);
			if (origin != null)
				AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
		}
	}

	public static String getString(ContentResolver resolver, String name) {
		LogUtil.i("hit name = " + name);
		if (name == Secure.ANDROID_ID) {
			return ANDROID_ID;
		}
		return (String) AndHook.callStaticObjectOrigin(Secure.class, resolver,
				name);
	}

	private static void fake_Secure_getString() {
		final String target = "getString";
		try {
			final Method origin = Secure.class.getDeclaredMethod(target,
					ContentResolver.class, String.class);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					ContentResolver.class, String.class);
			AndHook.hook(origin, fake);
		} catch (Exception e) {
			LogUtil.e(target, e);
		}
	}

	public static int getInt(ContentResolver resolver, String name, int def) {
		LogUtil.i("hit name = " + name + ", def = " + def);
		if (name == Secure.ALLOW_MOCK_LOCATION) {
			return 0;
		}
		try {
			def = (int) AndHook.callStaticIntOrigin(Secure.class, resolver,
					name, def);
		} catch (Exception e) {
			LogUtil.w(e);
		}
		return def;
	}

	public static int getInt2(ContentResolver resolver, String name) {
		LogUtil.i("hit name = " + name);
		if (name == Secure.ALLOW_MOCK_LOCATION) {
			return 0;
		}
		return (int) AndHook.callStaticIntOrigin(Secure.class, resolver, name);
	}

	private static void fake_Secure_getInt() {
		final String target = "getInt";
		try {
			final Method origin = Secure.class.getDeclaredMethod(target,
					ContentResolver.class, String.class, int.class);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					ContentResolver.class, String.class, int.class);
			AndHook.hook(origin, fake);
		} catch (Exception e) {
			LogUtil.e(target, e);
		}

		try {
			final Method origin = Secure.class.getDeclaredMethod(target,
					ContentResolver.class, String.class);
			final Method fake = PreHook.class.getDeclaredMethod(target + "2",
					ContentResolver.class, String.class);
			AndHook.hook(origin, fake);
		} catch (Exception e) {
			LogUtil.e(target + "2", e);
		}
	}

	public String getMacAddress() {
		LogUtil.i("hit");
		return MAC;
	}

	private static void fake_WifiInfo() {
		final String target = "getMacAddress";
		try {
			final Method origin = WifiInfo.class.getDeclaredMethod(target,
					(Class<?>[]) null);
			final Method fake = PreHook.class.getDeclaredMethod(target,
					(Class<?>[]) null);
			AndHook.hookNoBackup(origin, fake);
		} catch (Exception e) {
			LogUtil.e(target, e);
		}
	}

	public static void Init(ClassLoader patchClassLoader, Context context,
			String processName) {
		LogUtil.i("Begin " + processName);
		if (!processName.endsWith(":patch")) { // com.jingdong.app.mall:patch
			if (android.os.Build.VERSION.SDK_INT <= 19) {
				apk.andhook.DalvikHook.init();
			}

			fake_IMEI();
			fake_IMSI();
			fake_ICCID();
			fake_MSISDN();
			fake_SIM();
			if (context == null) {
				fake_Secure_getString();
				fake_Secure_getInt();
				fake_WifiInfo();
			}
		}
		LogUtil.i("End " + processName);
	}

}

package io.virtualhook;

import android.app.Application;
import apk.andhook.AndHook;

public class PostHook {

	public static void Init(ClassLoader patchClassLoader, Application app,
			String processName) {
		LogUtil.i("Begin " + processName);

		LogUtil.i("End " + processName);
	}
}
